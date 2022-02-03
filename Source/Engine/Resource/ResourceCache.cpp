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
#include "IO/FileWatcher.h"


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
        , autoReloadResources_(false)
        , returnFailedResources_(false)
        , searchPackagesFirst_(true)
        , isRouting_(false)
        , finishBackgroundResourcesMs_(5)
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

    bool ResourceCache::AddResourceDir(const String &pathName, unsigned int priority)
    {
        MutexLock lock(resourceMutex_);

        auto* fileSystem = GetSubsystem<FileSystem>();
        if (!fileSystem || !fileSystem->DirExists(pathName))
        {
            MY3D_LOGERROR("Could not open directory " + pathName);
            return false;
        }

        // Convert path to absolut
        String fixedPath = SanitateResourceDirName(pathName);

        // Check that the same path does not already exist
        for (unsigned i = 0; i < resourceDirs_.Size(); ++i)
        {
            if (!resourceDirs_[i].Compare(fixedPath, false))
                return true;
        }

        if (priority < resourceDirs_.Size())
            resourceDirs_.Insert(priority, fixedPath);
        else
            resourceDirs_.Push(fixedPath);

        // If resource auto-reloading active, create a file watcher for the directory
        if (autoReloadResources_)
        {
            SharedPtr<FileWatcher> watcher(new FileWatcher(context_));
            watcher->StartWatching(fixedPath, true);
            fileWatchers_.Push(watcher);
        }

        MY3D_LOGINFO("Added resource path " + fixedPath);
        return true;
    }

    bool ResourceCache::AddPackageFile(PackageFile* package, unsigned priority)
    {
        MutexLock lock(resourceMutex_);

        // Do not add packages that failed to load
        if (!package || !package->GetNumFiles())
        {
            MY3D_LOGERRORF("Could not add package file %s due to load failure", package->GetName().CString());
            return false;
        }

        if (priority < packages_.Size())
            packages_.Insert(priority, SharedPtr<PackageFile>(package));
        else
            packages_.Push(SharedPtr<PackageFile>(package));

        MY3D_LOGINFO("Added resource package " + package->GetName());
        return true;
    }

    bool ResourceCache::AddPackageFile(const String& fileName, unsigned priority)
    {
        SharedPtr<PackageFile> package(new PackageFile(context_));
        return package->Open(fileName) && AddPackageFile(package, priority);
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

    void ResourceCache::RemoveResourceDir(const String &pathName)
    {
        MutexLock lock(resourceMutex_);

        String fixedPath = SanitateResourceDirName(pathName);

        for (unsigned i = 0; i < resourceDirs_.Size(); ++i)
        {
            if (!resourceDirs_[i].Compare(fixedPath, false))
            {
                resourceDirs_.Erase(i);
                // Remove the filewatcher with the matching path
                for (unsigned j = 0; j < fileWatchers_.Size(); ++j)
                {
                    if (!fileWatchers_[j]->GetPath().Compare(fixedPath, false))
                    {
                        fileWatchers_.Erase(j);
                        break;
                    }
                }
                MY3D_LOGINFO("Removed resource path " + fixedPath);
                return;
            }
        }
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

    SharedPtr<Resource> ResourceCache::GetTempResource(StringHash type, const String& name, bool sendEventOnFailure)
    {
        String sanitatedName = SanitateResourceName(name);

        // If empty name, return null pointer immediately
        if (sanitatedName.Empty())
            return SharedPtr<Resource>();

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

            return SharedPtr<Resource>();
        }

        // Attempt to load the resource
        SharedPtr<File> file = GetFile(sanitatedName, sendEventOnFailure);
        if (!file)
            return SharedPtr<Resource>();  // Error is already logged

        MY3D_LOGERROR("Loading temporary resource " + sanitatedName);
        resource->SetName(file->GetName());

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

            return SharedPtr<Resource>();
        }

        return resource;
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

    void ResourceCache::ReleaseResource(StringHash type, const String& name, bool force)
    {
        StringHash nameHash(name);
        const SharedPtr<Resource>& existingRes = FindResource(type, nameHash);
        if (!existingRes)
            return;

        // If other references exist, do not release, unless forced
        if ((existingRes.Refs() == 1 && existingRes.WeakRefs() == 0) || force)
        {
            resourceGroups_[type].resources_.Erase(nameHash);
            UpdateResourceGroup(type);
        }
    }

    void ResourceCache::ReleaseResources(StringHash type, bool force)
    {
        bool released = false;

        HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
        if (i != resourceGroups_.End())
        {
            for (HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Begin();
                 j != i->second_.resources_.End();)
            {
                HashMap<StringHash, SharedPtr<Resource> >::Iterator current = j++;
                // If other references exist, do not release, unless forced
                if ((current->second_.Refs() == 1 && current->second_.WeakRefs() == 0) || force)
                {
                    i->second_.resources_.Erase(current);
                    released = true;
                }
            }
        }

        if (released)
            UpdateResourceGroup(type);
    }

    void ResourceCache::ReleaseResources(StringHash type, const String& partialName, bool force)
    {
        bool released = false;

        HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Find(type);
        if (i != resourceGroups_.End())
        {
            for (HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Begin();
                 j != i->second_.resources_.End();)
            {
                HashMap<StringHash, SharedPtr<Resource> >::Iterator current = j++;
                if (current->second_->GetName().Contains(partialName))
                {
                    // If other references exist, do not release, unless forced
                    if ((current->second_.Refs() == 1 && current->second_.WeakRefs() == 0) || force)
                    {
                        i->second_.resources_.Erase(current);
                        released = true;
                    }
                }
            }
        }

        if (released)
            UpdateResourceGroup(type);
    }

    void ResourceCache::ReleaseResources(const String& partialName, bool force)
    {
        // Some resources refer to others, like materials to textures. Repeat the release logic as many times as necessary to ensure
        // these get released. This is not necessary if forcing release
        bool released;
        do
        {
            released = false;

            for (HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Begin(); i != resourceGroups_.End(); ++i)
            {
                for (HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Begin();
                     j != i->second_.resources_.End();)
                {
                    HashMap<StringHash, SharedPtr<Resource> >::Iterator current = j++;
                    if (current->second_->GetName().Contains(partialName))
                    {
                        // If other references exist, do not release, unless forced
                        if ((current->second_.Refs() == 1 && current->second_.WeakRefs() == 0) || force)
                        {
                            i->second_.resources_.Erase(current);
                            released = true;
                        }
                    }
                }
                if (released)
                    UpdateResourceGroup(i->first_);
            }

        } while (released && !force);
    }

    void ResourceCache::ReleaseAllResources(bool force)
    {
        bool released;
        do
        {
            released = false;

            for (HashMap<StringHash, ResourceGroup>::Iterator i = resourceGroups_.Begin();
                 i != resourceGroups_.End(); ++i)
            {
                for (HashMap<StringHash, SharedPtr<Resource> >::Iterator j = i->second_.resources_.Begin();
                     j != i->second_.resources_.End();)
                {
                    HashMap<StringHash, SharedPtr<Resource> >::Iterator current = j++;
                    // If other references exist, do not release, unless forced
                    if ((current->second_.Refs() == 1 && current->second_.WeakRefs() == 0) || force)
                    {
                        i->second_.resources_.Erase(current);
                        released = true;
                    }
                }
                if (released)
                    UpdateResourceGroup(i->first_);
            }

        } while (released && !force);
    }

    bool ResourceCache::ReloadResource(Resource *resource)
    {
        if (!resource)
            return false;

        resource->SendEvent(E_RELOADSTARTED);

        bool success = false;
        SharedPtr<File> file = GetFile(resource->GetName());
        if (file)
            success = resource->Load(*(file.Get()));

        if (success)
        {
            resource->ResetUseTimer();
            UpdateResourceGroup(resource->GetType());
            resource->SendEvent(E_RELOADFINISHED);
            return true;
        }

        // If reloading failed, do not remove the resource from cache, to allow for a new live edit to
        // attempt loading again
        resource->SendEvent(E_RELOADFAILED);
        return false;
    }

    void ResourceCache::ReloadResourceWithDependencies(const String& fileName)
    {
        StringHash fileNameHash(fileName);
        // If the filename is a resource we keep track of, reload it
        const SharedPtr<Resource>& resource = FindResource(fileNameHash);
        if (resource)
        {
            MY3D_LOGDEBUG("Reloading changed resource " + fileName);
            ReloadResource(resource);
        }
        // Always perform dependency resource check for resource loaded from XML file as it could be used in inheritance
        if (!resource || GetExtension(resource->GetName()) == ".xml")
        {
            // Check if this is a dependency resource, reload dependents
            HashMap<StringHash, HashSet<StringHash> >::ConstIterator j = dependentResources_.Find(fileNameHash);
            if (j != dependentResources_.End())
            {
                // Reloading a resource may modify the dependency tracking structure. Therefore collect the
                // resources we need to reload first
                Vector<SharedPtr<Resource> > dependents;
                dependents.Reserve(j->second_.Size());

                for (HashSet<StringHash>::ConstIterator k = j->second_.Begin(); k != j->second_.End(); ++k)
                {
                    const SharedPtr<Resource>& dependent = FindResource(*k);
                    if (dependent)
                        dependents.Push(dependent);
                }

                for (unsigned k = 0; k < dependents.Size(); ++k)
                {
                    MY3D_LOGDEBUG("Reloading resource " + dependents[k]->GetName() + " depending on " + fileName);
                    ReloadResource(dependents[k]);
                }
            }
        }
    }

    void ResourceCache::SetMemoryBudget(StringHash type, unsigned long long budget)
    {
        resourceGroups_[type].memoryBudget_ = budget;
    }

    void ResourceCache::SetAutoReloadResources(bool enable)
    {
        if (enable != autoReloadResources_)
        {
            if (enable)
            {
                for (unsigned i = 0; i < resourceDirs_.Size(); ++i)
                {
                    SharedPtr<FileWatcher> watcher(new FileWatcher(context_));
                    watcher->StartWatching(resourceDirs_[i], true);
                    fileWatchers_.Push(watcher);
                }
            }
            else
                fileWatchers_.Clear();

            autoReloadResources_ = enable;
        }
    }

    void ResourceCache::AddResourceRouter(ResourceRouter* router, bool addAsFirst)
    {
        // Check for duplicate
        for (unsigned i = 0; i < resourceRouters_.Size(); ++i)
        {
            if (resourceRouters_[i] == router)
                return;
        }

        if (addAsFirst)
            resourceRouters_.Insert(0, SharedPtr<ResourceRouter>(router));
        else
            resourceRouters_.Push(SharedPtr<ResourceRouter>(router));
    }

    void ResourceCache::RemoveResourceRouter(ResourceRouter* router)
    {
        for (unsigned i = 0; i < resourceRouters_.Size(); ++i)
        {
            if (resourceRouters_[i] == router)
            {
                resourceRouters_.Erase(i);
                return;
            }
        }
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

    String ResourceCache::SanitateResourceDirName(const String &name) const
    {
        String fixedPath = AddTrailingSlash(name);
        if (!IsAbsolutePath(fixedPath))
            fixedPath = GetSubsystem<FileSystem>()->GetCurrentDir() + fixedPath;

        // Sanitate away /./ construct
        fixedPath.Replace("/./", "/");

        return fixedPath.Trimmed();
    }

    bool ResourceCache::BackgroundLoadResource(StringHash type, const String& name, bool sendEventOnFailure, Resource* caller)
    {
        // If empty name, fail immediately
        String sanitatedName = SanitateResourceName(name);
        if (sanitatedName.Empty())
            return false;

        // First check if already exists as a loaded resource
        StringHash nameHash(sanitatedName);
        if (FindResource(type, nameHash) != noResource)
            return false;

        return backgroundLoader_->QueueResource(type, sanitatedName, sendEventOnFailure, caller);
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
        for (unsigned i = 0; i < fileWatchers_.Size(); ++i)
        {
            String fileName;
            while (fileWatchers_[i]->GetNextChange(fileName))
            {
                ReloadResourceWithDependencies(fileName);
                // Finally send a general file changed event even if the file was not a tracked resource
                using namespace FileChanged;

                VariantMap& eventData = GetEventDataMap();
                eventData[P_FILENAME] = fileWatchers_[i]->GetPath() + fileName;
                eventData[P_RESOURCENAME] = fileName;
                SendEvent(E_FILECHANGED, eventData);
            }
        }

        // Check for background loaded resources that can be finished
        backgroundLoader_->FinishResources(finishBackgroundResourcesMs_);
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