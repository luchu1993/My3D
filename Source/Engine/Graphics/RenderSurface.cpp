//
// Created by luchu on 2022/1/22.
//

#include "Graphics/Texture.h"
#include "Graphics/RenderSurface.h"


namespace My3D
{

    int RenderSurface::GetWidth() const
    {
        return parentTexture_->GetWidth();
    }

    int RenderSurface::GetHeight() const
    {
        return parentTexture_->GetHeight();
    }
}