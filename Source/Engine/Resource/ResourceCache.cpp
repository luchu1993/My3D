//
// Created by luchu on 2022/1/13.
//

#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"
#include "Resource/BackgroundLoader.h"
#include "Core/CoreEvents.h"


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

    void RegisterResourceLibrary(Context* context)
    {
        XMLFile::RegisterObject(context);
    }

    void ResourceCache::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
    {
    }
}