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

    private:
        /// Handle begin frame event. Automatic resource reloads and the finalization of background loaded resources are processed here.
        void HandleBeginFrame(StringHash eventType, VariantMap& eventData);

        /// Mutex for thread-safe access to the resource directories, resource packages and resource dependencies.
        mutable Mutex resourceMutex_;
        /// Resources by type.
        HashMap<StringHash, ResourceGroup> resourceGroups_;
        /// Resource load directories.
        Vector<String> resourceDirs_;
        /// Dependent resources. Only used with automatic reload to eg. trigger reload of a cube texture when any of its faces change.
        HashMap<StringHash, HashSet<StringHash> > dependentResources_;
        /// Resource background loader.
        SharedPtr<BackgroundLoader> backgroundLoader_;
        /// Resource routers.
        Vector<SharedPtr<ResourceRouter> > resourceRouters_;
    };

    /// Register Resource library subsystems and objects.
    void MY3D_API RegisterResourceLibrary(Context* context);
}
