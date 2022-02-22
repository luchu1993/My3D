//
// Created by luchu on 2022/2/21.
//

#pragma once

#include "Container/Ptr.h"
#include "Container/RefCounted.h"
#include "Graphics/GraphicsDefs.h"
#include "Math/Color.h"
#include "Math/Vector4.h"


namespace My3D
{
    class XMLElement;
    class XMLFile;

    /// Rendering path command types.
    enum RenderCommandType
    {
        CMD_NONE = 0,
        CMD_CLEAR,
        CMD_SCENEPASS,
        CMD_QUAD,
        CMD_FORWARDLIGHTS,
        CMD_LIGHTVOLUMES,
        CMD_RENDERUI,
        CMD_SENDEVENT
    };

    /// Rendering path sorting modes.
    enum RenderCommandSortMode
    {
        SORT_FRONTTOBACK = 0,
        SORT_BACKTOFRONT
    };

    /// Rendertarget size mode.
    enum RenderTargetSizeMode
    {
        SIZE_ABSOLUTE = 0,
        SIZE_VIEWPORTDIVISOR,
        SIZE_VIEWPORTMULTIPLIER
    };
}