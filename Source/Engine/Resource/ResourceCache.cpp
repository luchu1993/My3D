//
// Created by luchu on 2022/1/13.
//

#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"
#include "Resource/BackgroundLoader.h"
#include "Core/CoreEvents.h"
#include "IO/FileSystem.h"
#include "IO/PackageFile.h"
#include "IO/Log.h"
#include "Resource/ResourceEvents.h"


namespace My3D
{
    ResourceCache::ResourceCache(Context *context)
        : Object(context)
    {
        // Register Resource library object factories
        RegisterResourceLibrary(context_);
        // Create resource background loader. Its thread will start on the first background request
        backgroundLoader_ = new BackgroundLoader(this);
        // Subscribe BeginFrame for handling directory watchers and background loaded resource finalization
        SubscribeToEvent(E_BEGINFRAME, MY3D_HANDLER(ResourceCache, HandleBeginFrame));
    }

    ResourceCache::~ResourceCache()
    {
        backgroundLoader_.Reset();
    }

    bool ResourceCache::AddManualResource(Resource *resource)
    {
        if (!resource)
        {
            MY3D_LOGERROR("Null manual resource");
            return false;
        }

        const String& name = resource->GetName();
        if (name.Empty())
        {
            MY3D_LOGERROR("Manual resource with empty name, can not add");
            return false;
        }

        resource->ResetUseTimer();
        resourceGroups_[resource->GetType()].resources_[resource->GetNameHash()] = resource;
        UpdateResourceGroup(resource->GetType());
        return true;
    }

    SharedPtr<File> ResourceCache::GetFile(const String &name, bool sendEventOnFailure)
    {
        MutexLock lock(resourceMutex_);

        String sanitatedName = SanitateResourceName(name);
        if (!isRouting_)
        {
            isRouting_ = true;
            for (unsigned i = 0; i < resourceRouters_.Size(); ++i)
                resourceRouters_[i]->Route(sanitatedName, RESOURCE_GETFILE);
            isRouting_ = false;
        }

        if (sanitatedName.Length())
        {
            File* file = nullptr;
            if (searchPackagesFirst_)
            {
                file = SearchPackages(sanitatedName);
                if (!file)
                    file = SearchResourceDirs(sanitatedName);
            }
            else
            {
                file = SearchResourceDirs(sanitatedName);
                if (!file)
                    file = SearchPackages(sanitatedName);
            }

            if (file)
                return SharedPtr<File>(file);
        }

        if (sendEventOnFailure)
        {
            if (resourceRouters_.Size() && sanitatedName.Empty() && !name.Empty())
                MY3D_LOGERROR("Resource request " + name + " was blocked");
            else
                MY3D_LOGERROR("Could not find resource " + sanitatedName);

            if (Thread::IsMainThread())
            {
                using namespace ResourceNotFound;

                VariantMap& eventData = GetEventDataMap();
                eventData[P_RESOURCENAME] = sanitatedName.Length() ? sanitatedName : name;
                SendEvent(E_RESOURCENOTFOUND, eventData);
            }
        }

        return SharedPtr<File>();
    }

    String ResourceCache::SanitateResourceName(const String &name) const
    {
        // Sanitate unsupported constructs from the resource name
        String sanitatedName = GetInternalPath(name);
        sanitatedName.Replace("../", "");
        sanitatedName.Replace("./", "");

        // If the path refers to one of the resource directories, normalize the resource name
        auto* fileSystem = GetSubsystem<FileSystem>();
        if (resourceDirs_.Size())
        {
            String namePath = GetPath(sanitatedName);
            String exePath = fileSystem->GetProgramDir().Replaced("/./", "/");
            for (unsigned i = 0; i < resourceDirs_.Size(); ++i)
            {
                String relativeResourcePath = resourceDirs_[i];
                if (relativeResourcePath.StartsWith(exePath))
                    relativeResourcePath = relativeResourcePath.Substring(exePath.Length());

                if (namePath.StartsWith(resourceDirs_[i], false))
                    namePath = namePath.Substring(resourceDirs_[i].Length());
                else if (namePath.StartsWith(relativeResourcePath, false))
                    namePath = namePath.Substring(relativeResourcePath.Length());
            }

            sanitatedName = namePath + GetFileNameAndExtension(sanitatedName);
        }

        return sanitatedName.Trimmed();
    }

    void RegisterResourceLibrary(Context* context)
    {
        XMLFile::RegisterObject(context);
    }

    void ResourceCache::UpdateResourceGroup(StringHash type)
    {
        HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
        if (i == resourceGroups_.End())
            return;

        for (;;)
        {
            unsigned totalSize = 0;
            unsigned oldestTimer = 0;
            HashMap<StringHash, SharedPtr<Resource> >::Iterator oldestResource = i->second_.resources_.End();

            for (HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Begin();
                 j != i->second_.resources_.End(); ++j)
            {
                totalSize += j->second_->GetMemoryUse();
                unsigned useTimer = j->second_->GetUseTimer();
                if (useTimer > oldestTimer)
                {
                    oldestTimer = useTimer;
                    oldestResource = j;
                }
            }

            i->second_.memoryUse_ = totalSize;

            // If memory budget defined and is exceeded, remove the oldest resource and loop again
            // (resources in use always return a zero timer and can not be removed)
            if (i->second_.memoryBudget_ && i->second_.memoryUse_ > i->second_.memoryBudget_ &&
                oldestResource != i->second_.resources_.End())
            {
                MY3D_LOGDEBUG("Resource group " + oldestResource->second_->GetTypeName() + " over memory budget, releasing resource " +
                                oldestResource->second_->GetName());
                i->second_.resources_.Erase(oldestResource);
            }
            else
                break;
        }
    }

    void ResourceCache::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
    {
    }

    File *ResourceCache::SearchResourceDirs(const String &name)
    {
        auto* fileSystem = GetSubsystem<FileSystem>();
        for (unsigned i = 0; i < resourceDirs_.Size(); ++i)
        {
            if (fileSystem->FileExists(resourceDirs_[i] + name))
            {
                // Construct the file first with full path, then rename it to not contain the resource path,
                // so that the file's sanitatedName can be used in further GetFile() calls (for example over the network)
                File* file(new File(context_, resourceDirs_[i] + name));
                file->SetName(name);
                return file;
            }
        }

        // Fallback using absolute path
        if (fileSystem->FileExists(name))
            return new File(context_, name);

        return nullptr;
    }

    File *ResourceCache::SearchPackages(const String &name)
    {
        for (unsigned i = 0; i < packages_.Size(); ++i)
        {
            if (packages_[i]->Exists(name))
                return new File(context_, packages_[i], name);
        }

        return nullptr;
    }
}