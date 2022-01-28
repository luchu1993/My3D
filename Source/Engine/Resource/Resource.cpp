//
// Created by luchu on 2022/1/13.
//

#include "Resource/Resource.h"
#include "IO/Log.h"
#include "IO/File.h"
#include "Core/Thread.h"


namespace My3D
{
    Resource::Resource(Context *context)
        : Object(context)
        , memoryUse_(0)
        , asyncLoadState_(ASYNC_DONE)
    {

    }

    bool Resource::Load(Deserializer &source)
    {
        SetAsyncLoadState(Thread::IsMainThread() ? ASYNC_DONE : ASYNC_LOADING);

        bool success = BeginLoad(source);
        if (success)
            success &= EndLoad();

        SetAsyncLoadState(ASYNC_DONE);

        return success;
    }

    bool Resource::BeginLoad(Deserializer& source)
    {
        // This always needs to be overridden by subclasses
        return false;
    }

    bool Resource::EndLoad()
    {
        // If no GPU upload step is necessary, no override is necessary
        return true;
    }

    bool Resource::Save(Serializer &dest) const
    {
        MY3D_LOGERROR("Save not supported for " + GetTypeName());
        return false;
    }
    void Resource::SetName(const String& name)
    {
        name_ = name;
        nameHash_ = name;
    }

    void Resource::SetMemoryUse(unsigned size)
    {
        memoryUse_ = size;
    }

    void Resource::ResetUseTimer()
    {
        useTimer_.Reset();
    }

    void Resource::SetAsyncLoadState(AsyncLoadState newState)
    {
        asyncLoadState_ = newState;
    }

    unsigned Resource::GetUseTimer()
    {
        // If more references than the resource cache, return always 0 & reset the timer
        if (Refs() > 1)
        {
            useTimer_.Reset();
            return 0;
        }
        else
            return useTimer_.GetMSec(false);
    }

    bool Resource::LoadFile(const String& fileName)
    {
        File file(context_);
        return file.Open(fileName, FILE_READ) && Load(file);
    }

    bool Resource::SaveFile(const String& fileName) const
    {
        File file(context_);
        return file.Open(fileName, FILE_WRITE) && Save(file);
    }

}