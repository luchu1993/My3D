//
// Created by luchu on 2022/2/19.
//


#include "Core/Context.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture3D.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"


namespace My3D
{

    Texture3D::Texture3D(Context *context)
        : Texture(context)
    {
#ifdef MY3D_OPENGL
        target_ = GL_TEXTURE_3D;
#endif
    }

    Texture3D::~Texture3D()
    {
        Release();
    }

    void Texture3D::RegisterObject(Context* context)
    {
        context->RegisterFactory<Texture3D>();
    }

    bool Texture3D::BeginLoad(Deserializer &source)
    {
        return true;
    }

    bool Texture3D::EndLoad()
    {
        return true;
    }

    bool Texture3D::SetSize(int width, int height, int depth, unsigned format, TextureUsage usage)
    {
        if (width <= 0 || height <= 0 || depth <= 0)
        {
            MY3D_LOGERROR("Zero or negative 3D texture dimensions");
            return false;
        }
        if (usage >= TEXTURE_RENDERTARGET)
        {
            MY3D_LOGERROR("Rendertarget or depth-stencil usage not supported for 3D textures");
            return false;
        }

        usage_ = usage;

        width_ = width;
        height_ = height;
        depth_ = depth;
        format_ = format;

        return Create();
    }
}

