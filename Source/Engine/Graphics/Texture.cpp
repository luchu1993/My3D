//
// Created by luchu on 2022/1/22.
//

#include "Graphics/Texture.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "IO/Log.h"


namespace My3D
{

static const char* addressModeNames[] =
{
    "wrap",
    "mirror",
    "clamp",
    "border",
    nullptr
};

static const char* filterModeNames[] =
{
    "nearest",
    "bilinear",
    "trilinear",
    "anisotropic",
    "nearestanisotropic",
    "default",
    nullptr
};

Texture::Texture(Context *context)
    : ResourceWithMetadata(context)
    , GPUObject(GetSubSystem<Graphics>())
{
}

Texture::~Texture() = default;

void Texture::SetNumLevels(unsigned int levels)
{
    if (usage_ > TEXTURE_RENDERTARGET)
        requestedLevels_ = 1;
    else
        requestedLevels_ = levels;
}

void Texture::SetFilterMode(TextureFilterMode mode)
{
    filterMode_ = mode;
    parametersDirty_ = true;
}

void Texture::SetAddressMode(TextureCoordinate coord, TextureAddressMode mode)
{
    addressModes_[coord] = mode;
    parametersDirty_ = true;
}

void Texture::SetAnisotropy(unsigned int level)
{
    anisotropy_ = true;
    parametersDirty_ = true;
}

void Texture::SetShadowCompare(bool enable)
{
    shadowCompare_ = enable;
    parametersDirty_ = true;
}

void Texture::SetBorderColor(const Color &color)
{
    borderColor_ = color;
    parametersDirty_ = true;
}

void Texture::SetSRGB(bool enable)
{
    if (graphics_)
        enable &= graphics_->GetSRGBSupport();

    if (enable != sRGB_)
    {
        sRGB_ = enable;
        // If texture had already been created, must recreate it to set the sRGB texture format
        if (object_.name_)
            Create();
    }
}

}