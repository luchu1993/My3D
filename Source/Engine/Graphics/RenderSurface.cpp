//
// Created by luchu on 2022/1/22.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Texture.h"


namespace My3D
{
    RenderSurface::~RenderSurface()
    {
        Release();
    }

    void RenderSurface::SetNumViewports(unsigned num)
    {
        viewports_.Resize(num);
    }

    void RenderSurface::SetViewport(unsigned index, Viewport* viewport)
    {
        if (index >= viewports_.Size())
            viewports_.Resize(index + 1);

        viewports_[index] = viewport;
    }

    void RenderSurface::SetUpdateMode(RenderSurfaceUpdateMode mode)
    {
        updateMode_ = mode;
    }

    void RenderSurface::SetLinkedRenderTarget(RenderSurface* renderTarget)
    {
        if (renderTarget != this)
            linkedRenderTarget_ = renderTarget;
    }

    void RenderSurface::SetLinkedDepthStencil(RenderSurface* depthStencil)
    {
        if (depthStencil != this)
            linkedDepthStencil_ = depthStencil;
    }

    void RenderSurface::QueueUpdate()
    {
        updateQueued_ = true;
    }

    void RenderSurface::ResetUpdateQueued()
    {
        updateQueued_ = false;
    }

    int RenderSurface::GetWidth() const
    {
        return parentTexture_->GetWidth();
    }

    int RenderSurface::GetHeight() const
    {
        return parentTexture_->GetHeight();
    }

    TextureUsage RenderSurface::GetUsage() const
    {
        return parentTexture_->GetUsage();
    }

    int RenderSurface::GetMultiSample() const
    {
        return parentTexture_->GetMultiSample();
    }

    bool RenderSurface::GetAutoResolve() const
    {
        return parentTexture_->GetAutoResolve();
    }

    Viewport* RenderSurface::GetViewport(unsigned index) const
    {
        return index < viewports_.Size() ? viewports_[index] : nullptr;
    }
}