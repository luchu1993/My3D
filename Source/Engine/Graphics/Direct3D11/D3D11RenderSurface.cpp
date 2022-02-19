//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Texture.h"


namespace My3D
{
    RenderSurface::RenderSurface(Texture *parentTexture)
        : parentTexture_(parentTexture)
        , renderTargetView_(nullptr)
        , readOnlyView_(nullptr)
    {
    }

    void RenderSurface::Release()
    {
        Graphics* graphics = parentTexture_->GetGraphics();
        if (graphics && renderTargetView_)
        {
            for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
            {
                if (graphics->GetRenderTarget(i) == this)
                    graphics->ResetRenderTarget(i);
            }

            if (graphics->GetDepthStencil() == this)
                graphics->ResetDepthStencil();
        }

        MY3D_SAFE_RELEASE(renderTargetView_);
        MY3D_SAFE_RELEASE(readOnlyView_);
    }

    bool RenderSurface::CreateRenderBuffer(unsigned width, unsigned height, unsigned format, int multiSample)
    {
        // Not used on Direct3D
        return false;
    }

    void RenderSurface::OnDeviceLost()
    {
        // No-op on Direct3D
    }

}