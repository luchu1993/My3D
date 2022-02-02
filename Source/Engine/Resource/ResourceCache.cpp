//
// Created by luchu on 2022/1/13.
//

#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"
#include "Resource/BackgroundLoader.h"
#include "Core/CoreEvents.h"
#include "Core/Context.h"
#include "IO/FileSystem.h"
#include "IO/PackageFile.h"
#include "IO/Log.h"
#include "Resource/ResourceEvents.h"


namespace My3D
{
    static const char* checkDirs[] =
    {
        "Fonts",
        "Materials",
        "Models",
        "Music",
        "Objects",
        "Particle",
        "PostProcess",
        "RenderPaths",
        "Scenes",
        "Scripts",
        "Sounds",
        "Shaders",
        "Techniques",
        "Textures",
        "UI",
        nullptr
    };

    static const SharedPtr<Resource> noResource;

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

    Resource* ResourceCache::GetResource(StringHash type, const String& name, bool sendEventOnFailure)
    {
        String sanitatedName = SanitateResourceName(name);

        if (!Thread::IsMainThread())
        {
            MY3D_LOGERROR("Attempted to get resource " + sanitatedName + " from outside the main thread");
            return nullptr;
        }

        // If empty name, return null pointer immediately
        if (sanitatedName.Empty())
            return nullptr;

        StringHash nameHash(sanitatedName);

        // Check if the resource is being background loaded but is now needed immediately
        backgroundLoader_->WaitForResource(type, nameHash);

        const SharedPtr<Resource>& existing = FindResource(type, nameHash);
        if (existing)
            return existing;

        SharedPtr<Resource> resource;
        // Make sure the pointer is non-null and is a Resource subclass
        resource = DynamicCast<Resource>(context_->CreateObject(type));
        if (!resource)
        {
            MY3D_LOGERROR("Could not load unknown resource type " + String(type));

            if (sendEventOnFailure)
            {
                using namespace UnknownResourceType;

                VariantMap& eventData = GetEventDataMap();
                eventData[P_RESOURCETYPE] = type;
                SendEvent(E_UNKNOWNRESOURCETYPE, eventData);
            }

            return nullptr;
        }

        // Attempt to load the resource
        SharedPtr<File> file = GetFile(sanitatedName, sendEventOnFailure);
        if (!file)
            return nullptr;   // Error is already logged

        MY3D_LOGDEBUG("Loading resource " + sanitatedName);
        resource->SetName(sanitatedName);

        if (!resource->Load(*(file.Get())))
        {
            // Error should already been logged by corresponding resource descendant class
            if (sendEventOnFailure)
            {
                using namespace LoadFailed;

                VariantMap& eventData = GetEventDataMap();
                eventData[P_RESOURCENAME] = sanitatedName;
                SendEvent(E_LOADFAILED, eventData);
            }

            if (!returnFailedResources_)
                return nullptr;
        }

        // Store to cache
        resource->ResetUseTimer();
        resourceGroups_[type].resources_[nameHash] = resource;
        UpdateResourceGroup(type);

        return resource;
    }

    void ResourceCache::GetResources(PODVector<Resource *>& result, StringHash type) const
    {
        result.Clear();
        HashMap<StringHash, ResourceGroup>::ConstIterator i = resourceGroups_.Find(type);
        if (i != resourceGroups_.End())
        {
            for (HashMap<StringHash, SharedPtr<Resource> >::ConstIterator j = i->second_.resources_.Begin();
                 j != i->second_.resources_.End(); ++j)
                result.Push(j->second_);
        }
    }

    Resource* ResourceCache::GetExistingResource(StringHash type, const String& name)
    {
        String sanitatedName = SanitateResourceName(name);

        if (!Thread::IsMainThread())
        {
            MY3D_LOGERROR("Attempted to get resource " + sanitatedName + " from outside the main thread");
            return nullptr;
        }

        // If empty name, return null pointer immediately
        if (sanitatedName.Empty())
            return nullptr;

        StringHash nameHash(sanitatedName);

        const SharedPtr<Resource>& existing = FindResource(type, nameHash);
        return existing;
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

    const SharedPtr<Resource>& ResourceCache::FindResource(StringHash type, StringHash nameHash)
    {
        MutexLock lock(resourceMutex_);

        HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
        if (i == resourceGroups_.End())
            return noResource;
        HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Find(nameHash);
        if (j == i->second_.resources_.End())
            return noResource;

        return j->second_;
    }

    const SharedPtr<Resource>& ResourceCache::FindResource(StringHash nameHash)
    {
        MutexLock lock(resourceMutex_);

        for (HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Begin(); i != resourceGroups_.End(); ++i)
        {
            HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Find(nameHash);
            if (j != i->second_.resources_.End())
                return j->second_;
        }

        return noResource;
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

    void RegisterResourceLibrary(Context* context)
    {
        XMLFile::RegisterObject(context);
    }
}