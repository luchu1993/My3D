//
// Created by luchu on 2022/2/13.
//

#include "Graphics/Drawable.h"

namespace My3D
{
    const BoundingBox& Drawable::GetWorldBoundingBox()
    {
        if (worldBoundingBoxDirty_)
        {
            OnWorldBoundingBoxUpdate();
            worldBoundingBoxDirty_ = false;
        }

        return worldBoundingBox_;
    }

}