//
// Created by luchu on 2022/1/13.
//

#pragma once

#include "Container/List.h"
#include "Container/HashSet.h"
#include "Core/Mutex.h"
#include "IO/File.h"
#include "Resource/Resource.h"


namespace My3D
{

    class BackgroundLoader;
    class PackageFile;
    class FileWatcher;

    /// Sets to priority so that a package or file is pushed to the end of the vector.
    static const unsigned PRIORITY_LAST = 0xffffffff;

     /// Container of resources with specific type.
    struct ResourceGroup
    {
        /// Construct with defaults.
        ResourceGroup()
            : memoryBudget_(0)
            , memoryUse_(0)
        {
        }

        /// Memory budget.
        unsigned long long memoryBudget_;
        /// Current memory use.
        unsigned long long memoryUse_;
        /// Resources.
        HashMap<StringHash, SharedPtr<Resource> > resources_;
    };

    /// Resource request types.
    enum ResourceRequest
    {
        RESOURCE_CHECKEXISTS = 0,
        RESOURCE_GETFILE = 1
    };

    /// Optional resource request processor. Can deny requests, re-route resource file names, or perform other processing per request.
    class MY3D_API ResourceRouter : public Object
    {
    public:
        /// Construct.
        explicit ResourceRouter(Context* context)
            : Object(context)
        {
        }

        /// Process the resource request and optionally modify the resource name string. Empty name string means the resource is not found or not allowed.
        virtual void Route(String& name, ResourceRequest requestType) = 0;
    };

    /// Resource cache subsystem. Loads resources on demand and stores them for later access.
    class MY3D_API ResourceCache : public Object
    {
        MY3D_OBJECT(ResourceCache, Object)

    public:
        /// Construct.
        explicit ResourceCache(Context* context);
        /// Destruct. Free all resources.
        ~ResourceCache() override;
        /// Add a manually created resource. Must be uniquely named within its type.
        bool AddManualResource(Resource* resource);
        /// Open and return a file from the resource load paths or from inside a package file. If not found, use a fallback search with absolute path. Return null if fails. Can be called from outside the main thread.
        SharedPtr<File> GetFile(const String& name, bool sendEventOnFailure = true);
        /// Remove unsupported constructs from the resource name to prevent ambiguity, and normalize absolute filename to resource path relative if possible.
        String SanitateResourceName(const String& name) const;
        /// Return whether automatic resource reloading is enabled.
        bool GetAutoReloadResources() const { return autoReloadResources_; }
        /// Return whether resources that failed to load are returned.
        bool GetReturnFailedResources() const { return returnFailedResources_; }
        /// Return whether when getting resources should check package files or directories first.
        bool GetSearchPackagesFirst() const { return searchPackagesFirst_; }

    private:
        /// Update a resource group. Recalculate memory use and release resources if over memory budget.
        void UpdateResourceGroup(StringHash type);
        /// Handle begin frame event. Automatic resource reloads and the finalization of background loaded resources are processed here.
        void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
        /// Search FileSystem for file.
        File* SearchResourceDirs(const String& name);
        /// Search resource packages for file.
        File* SearchPackages(const String& name);

        /// Mutex for thread-safe access to the resource directories, resource packages and resource dependencies.
        mutable Mutex resourceMutex_;
        /// Resources by type.
        HashMap<StringHash, ResourceGroup> resourceGroups_;
        /// Resource load directories.
        Vector<String> resourceDirs_;
        /// Package files.
        Vector<SharedPtr<PackageFile> > packages_;
        /// Dependent resources. Only used with automatic reload to eg. trigger reload of a cube texture when any of its faces change.
        HashMap<StringHash, HashSet<StringHash> > dependentResources_;
        /// Resource background loader.
        SharedPtr<BackgroundLoader> backgroundLoader_;
        /// Resource routers.
        Vector<SharedPtr<ResourceRouter> > resourceRouters_;
        /// Automatic resource reloading flag.
        bool autoReloadResources_;
        /// Return failed resources flag.
        bool returnFailedResources_;
        /// Search priority flag.
        bool searchPackagesFirst_;
        /// Resource routing flag to prevent endless recursion.
        mutable bool isRouting_;
    };

    /// Register Resource library subsystems and objects.
    void MY3D_API RegisterResourceLibrary(Context* context);
}
