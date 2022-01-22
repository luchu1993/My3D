//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Graphics/GPUObject.h"
#include "Graphics/GraphicsDefs.h"
#include "Math/Color.h"
#include "Resource/Resource.h"


namespace My3D
{
    static const int MAX_TEXTURE_QUALITY_LEVELS = 3;

    /// Base class for texture resource

    class MY3D_API Texture : public ResourceWithMetadata, public GPUObject
    {
    public:
        /// Construct.
        explicit Texture(Context* context);
        /// Destruct.
        ~Texture() override;

        /// Set number of requested mip levels. Needs to be called before setting size.
        /** The default value (0) allocates as many mip levels as necessary to reach 1x1 size. Set value 1 to disable mipmapping.
            Note that rendertargets need to regenerate mips dynamically after rendering, which may cost performance. Screen buffers
            and shadow maps allocated by Renderer will have mipmaps disabled.
         */
        void SetNumLevels(unsigned levels);
        /// Set filtering mode.
        void SetFilterMode(TextureFilterMode mode);
        /// Set addressing mode by texture coordinate.
        void SetAddressMode(TextureCoordinate coord, TextureAddressMode mode);
        /// Set texture max. anisotropy level. No effect if not using anisotropic filtering. Value 0 (default) uses the default setting from Renderer.
        void SetAnisotropy(unsigned level);
        /// Set shadow compare mode. Not used on Direct3D9.
        void SetShadowCompare(bool enable);
        /// Set border color for border addressing mode.
        void SetBorderColor(const Color& color);
        /// Set sRGB sampling and writing mode.
        void SetSRGB(bool enable);
        /// Return API-specific texture format.
        unsigned GetFormat() const { return format_; }
        /// Return number of mip levels.
        unsigned GetLevels() const { return levels_; }
        /// Return width.
        int GetWidth() const { return width_; }
        /// Return height.
        int GetHeight() const { return height_; }
        /// Return depth.
        int GetDepth() const { return depth_; }
        /// Return texture usage type.
        TextureUsage GetUsage() const { return usage_; }

    protected:
        /// Create the GPU texture. Implemented in subclasses.
        virtual bool Create() { return true; }
        /// OpenGL target.
        unsigned target_{};
        /// Direct3D11 shader resource view.
        void* shaderResourceView_{};
        /// Direct3D11 sampler state object.
        void* sampler_{};
        /// Direct3D11 resolve texture object when multisample with autoresolve is used.
        void* resolveTexture_{};
        /// Texture format.
        unsigned format_{};
        /// Texture usage type.
        TextureUsage usage_{TEXTURE_STATIC};
        /// Current mip levels.
        unsigned levels_{};
        /// Requested mip levels.
        unsigned requestedLevels_{};
        /// Texture width.
        int width_{};
        /// Texture height.
        int height_{};
        /// Texture depth.
        int depth_{};
        /// Shadow compare mode.
        bool shadowCompare_{};
        /// Filtering mode.
        TextureFilterMode filterMode_{FILTER_DEFAULT};
        /// Addressing mode.
        TextureAddressMode addressModes_[MAX_COORDS]{ADDRESS_WRAP, ADDRESS_WRAP, ADDRESS_WRAP};
        /// Texture anisotropy level.
        unsigned anisotropy_{};
        /// Mip levels to skip when loading per texture quality setting.
        unsigned mipsToSkip_[MAX_TEXTURE_QUALITY_LEVELS]{2, 1, 0};
        /// Border color.
        Color borderColor_;
        /// Multisampling level.
        int multiSample_{1};
        /// sRGB sampling and writing mode flag.
        bool sRGB_{};
        /// Parameters dirty flag.
        bool parametersDirty_{true};
    };
}
