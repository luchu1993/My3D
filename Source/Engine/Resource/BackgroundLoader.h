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

private:
    /// Resource cache.
    ResourceCache* owner_;
    /// Mutex for thread-safe access to the background load queue.
    mutable Mutex backgroundLoadMutex_;
};
}
