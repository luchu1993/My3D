//
// Created by luchu on 2022/2/19.
//

#include "Core/Context.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/TextureCube.h"
#include "IO/FileSystem.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"


namespace My3D
{
    static const char* cubeMapLayoutNames[] = {
        "horizontal",
        "horizontalnvidia",
        "horizontalcross",
        "verticalcross",
        "blender",
        nullptr
    };

    static SharedPtr<Image> GetTileImage(Image* src, int tileX, int tileY, int tileWidth, int tileHeight)
    {
        return SharedPtr<Image>(
                src->GetSubimage(IntRect(tileX * tileWidth, tileY * tileHeight, (tileX + 1) * tileWidth, (tileY + 1) * tileHeight)));
    }

    TextureCube::TextureCube(Context* context)
        : Texture(context)
    {
#ifdef MY3D_OPENGL
        target_ = GL_TEXTURE_CUBE_MAP;
#endif
        // Default to clamp mode addressing
        addressModes_[COORD_U] = ADDRESS_CLAMP;
        addressModes_[COORD_V] = ADDRESS_CLAMP;
        addressModes_[COORD_W] = ADDRESS_CLAMP;
    }

    TextureCube::~TextureCube()
    {
        Release();
    }

    void TextureCube::RegisterObject(Context* context)
    {
        context->RegisterFactory<TextureCube>();
    }

    bool TextureCube::BeginLoad(Deserializer& source)
    {
        return true;
    }

    bool TextureCube::EndLoad()
    {
        return true;
    }
}