//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/ShaderPrecache.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"


namespace My3D
{
    ShaderPrecache::ShaderPrecache(Context* context, const String& fileName)
        : Object(context)
        , fileName_(fileName)
        , xmlFile_(context)
    {

    }

    ShaderPrecache::~ShaderPrecache()
    {
        MY3D_LOGINFO("End dumping shaders");

        if (usedCombinations_.Empty())
            return;

        File dest(context_, fileName_, FILE_WRITE);
        xmlFile_.Save(dest);
    }
}