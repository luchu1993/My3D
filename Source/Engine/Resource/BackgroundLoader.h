//
// Created by luchu on 2022/1/27.
//

#pragma once

#include "Container/HashMap.h"
#include "Container/HashSet.h"
#include "Container/RefCounted.h"
#include "Container/Ptr.h"
#include "Core/Mutex.h"
#include "Core/StringHash.h"
#include "Core/Thread.h"


namespace My3D
{
class Resource;
class ResourceCache;

/// Queue item for background loading of a resource
struct BackgroundLoadItem
{
    /// Resource
    SharedPtr<Resource> resource_;
    /// Resource depended on for loading
    HashSet<Pair<StringHash, StringHash>> dependencies_;
    /// Resource that depend on this resource's loading
    HashSet<Pair<StringHash, StringHash>> dependents_;
    /// Whether to send failure event
    bool sendEventOnFailure_;
};

/// Background loader of resources. Owned by the ResourceCache.
class BackgroundLoader : public RefCounted, public Thread
{
public:
    /// Construct.
    explicit BackgroundLoader(ResourceCache* owner);
    /// Destruct. Forcibly clear the load queue.
    ~BackgroundLoader() override;
    /// Resource background loading loop.
    void ThreadFunction() override;
    /// Queue loading of a resource.
    bool QueueResource(StringHash type, const String& name, bool sendEventOnFailure, Resource* caller);
    /// Wait and finish possible loading of a resource when being requested from the cache
    void WaitForResource(StringHash type, StringHash nameHash);
    /// Process resource that are ready to finish
    void FinishResources(int maxMs);
    /// Return amount of resources in the load queue
    unsigned GetNumQueuedResources() const;

private:
    /// Finish one background loaded resource
    void FinishBackgroundLoading(BackgroundLoadItem& item);

    /// Resource cache.
    ResourceCache* owner_;
    /// Mutex for thread-safe access to the background load queue.
    mutable Mutex backgroundLoadMutex_;
    /// Resource that are queued for background loading
    HashMap<Pair<StringHash, StringHash>, BackgroundLoadItem> backgroundLoadQueue_;
};
}
