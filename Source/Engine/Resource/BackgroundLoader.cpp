//
// Created by luchu on 2022/1/27.
//

#include "Resource/BackgroundLoader.h"

namespace My3D
{
    BackgroundLoader::BackgroundLoader(ResourceCache *owner)
        : owner_(owner)
    {
    }

    BackgroundLoader::~BackgroundLoader()
    {
        MutexLock lock(backgroundLoadMutex_);
    }

    void BackgroundLoader::ThreadFunction()
    {
    }
}