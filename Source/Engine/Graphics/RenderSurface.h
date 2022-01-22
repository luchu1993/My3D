//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Graphics/GraphicsDefs.h"
#include "Container/RefCounted.h"
#include "Graphics/Viewport.h"


namespace My3D
{
    class Texture;

    /// Color or depth-stencil surface that can be rendered into
    class MY3D_API RenderSurface : public RefCounted
    {
        friend class Texture2D;
        friend class Texture2DArray;
        friend class TextureCube;

    public:
        /// Construct
        explicit RenderSurface(Texture* parentTexture);
        /// Destruct
        ~RenderSurface() override;

        /// Set number of viewports
        void SetNumViewports(unsigned num);
        /// Set viewport
        void SetViewport(unsigned index, Viewport* viewport);

        /// Return width
        int GetWidth() const;
        /// Return height
        int GetHeight() const;
        /// Return parent texture.
        Texture* GetParentTexture() const { return parentTexture_; }

    private:
        /// Parent texture
        Texture* parentTexture_;
        /// Viewports
        Vector<SharedPtr<Viewport>> viewports_;

    };
}
