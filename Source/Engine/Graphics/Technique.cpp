//
// Created by luchu on 2022/2/19.
//

#include "Core/Context.h"
#include "Core/ProcessUtils.h"
#include "Graphics/Graphics.h"
#include "Graphics/Technique.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"
#include "Resource/XMLFile.h"


namespace My3D
{
    extern const char* cullModeNames[];

    const char* blendModeNames[] =
    {
        "replace",
        "add",
        "multiply",
        "alpha",
        "addalpha",
        "premulalpha",
        "invdestalpha",
        "subtract",
        "subtractalpha",
        nullptr
    };

    static const char* compareModeNames[] =
    {
        "always",
        "equal",
        "notequal",
        "less",
        "lessequal",
        "greater",
        "greaterequal",
        nullptr
    };

    static const char* lightingModeNames[] =
    {
        "unlit",
        "pervertex",
        "perpixel",
        nullptr
    };

    Pass::~Pass() = default;


    unsigned Technique::basePassIndex = 0;
    unsigned Technique::alphaPassIndex = 0;
    unsigned Technique::materialPassIndex = 0;
    unsigned Technique::deferredPassIndex = 0;
    unsigned Technique::lightPassIndex = 0;
    unsigned Technique::litBasePassIndex = 0;
    unsigned Technique::litAlphaPassIndex = 0;
    unsigned Technique::shadowPassIndex = 0;

    HashMap<String, unsigned> Technique::passIndices;

    Technique::Technique(Context* context)
        : Resource(context)
        , isDesktop_(false)
    {
        desktopSupport_ = true;
    }

    Technique::~Technique() = default;

    void Technique::RegisterObject(Context* context)
    {
        context->RegisterFactory<Technique>();
    }
}