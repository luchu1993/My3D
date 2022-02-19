//
// Created by luchu on 2022/2/19.
//

#include "Core/Context.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture2DArray.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"


namespace My3D
{
    Texture2DArray::Texture2DArray(Context *context)
        : Texture(context)
    {
#ifdef MY3D_OPENGL
        target_ = GL_TEXTURE_2D_ARRAY;
#endif
    }

    Texture2DArray::~Texture2DArray()
    {
        Release();
    }

    void Texture2DArray::RegisterObject(Context* context)
    {
        context->RegisterFactory<Texture2DArray>();
    }

    bool Texture2DArray::BeginLoad(Deserializer& source)
    {
        return true;
    }

    bool Texture2DArray::EndLoad()
    {
        return true;
    }
}

