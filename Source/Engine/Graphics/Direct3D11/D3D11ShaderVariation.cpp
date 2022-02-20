//
// Created by luchu on 2022/2/20.
//

#include <d3dcompiler.h>

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Shader.h"
#include "Graphics/VertexBuffer.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"


namespace My3D
{
    const char* ShaderVariation::elementSemanticNames[] =
    {
        "POSITION",
        "NORMAL",
        "BINORMAL",
        "TANGENT",
        "TEXCOORD",
        "COLOR",
        "BLENDWEIGHT",
        "BLENDINDICES",
        "OBJECTINDEX"
    };
}