//
// Created by luchu on 2022/1/18.
//

#pragma once

#include "Core/Object.h"

namespace My3D
{
/// New screen mode set.
MY3D_EVENT(E_SCREENMODE, ScreenMode)
{
    MY3D_PARAM(P_WIDTH, Width);                  // int
    MY3D_PARAM(P_HEIGHT, Height);                // int
    MY3D_PARAM(P_FULLSCREEN, Fullscreen);        // bool
    MY3D_PARAM(P_BORDERLESS, Borderless);        // bool
    MY3D_PARAM(P_RESIZABLE, Resizable);          // bool
    MY3D_PARAM(P_HIGHDPI, HighDPI);              // bool
    MY3D_PARAM(P_MONITOR, Monitor);              // int
    MY3D_PARAM(P_REFRESHRATE, RefreshRate);      // int
}

/// Window position changed.
MY3D_EVENT(E_WINDOWPOS, WindowPos)
{
    MY3D_PARAM(P_X, X);                          // int
    MY3D_PARAM(P_Y, Y);                          // int
}

/// Request for queuing rendersurfaces either in manual or always-update mode.
MY3D_EVENT(E_RENDERSURFACEUPDATE, RenderSurfaceUpdate)
{
}

/// Frame rendering started.
MY3D_EVENT(E_BEGINRENDERING, BeginRendering)
{
}

/// Frame rendering ended.
MY3D_EVENT(E_ENDRENDERING, EndRendering)
{
}

/// Update of a view started.
MY3D_EVENT(E_BEGINVIEWUPDATE, BeginViewUpdate)
{
    MY3D_PARAM(P_VIEW, View);                    // View pointer
    MY3D_PARAM(P_TEXTURE, Texture);              // Texture pointer
    MY3D_PARAM(P_SURFACE, Surface);              // RenderSurface pointer
    MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
    MY3D_PARAM(P_CAMERA, Camera);                // Camera pointer
}

/// Update of a view ended.
MY3D_EVENT(E_ENDVIEWUPDATE, EndViewUpdate)
{
    MY3D_PARAM(P_VIEW, View);                    // View pointer
    MY3D_PARAM(P_TEXTURE, Texture);              // Texture pointer
    MY3D_PARAM(P_SURFACE, Surface);              // RenderSurface pointer
    MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
    MY3D_PARAM(P_CAMERA, Camera);                // Camera pointer
}

/// Render of a view started.
MY3D_EVENT(E_BEGINVIEWRENDER, BeginViewRender)
{
    MY3D_PARAM(P_VIEW, View);                    // View pointer
    MY3D_PARAM(P_TEXTURE, Texture);              // Texture pointer
    MY3D_PARAM(P_SURFACE, Surface);              // RenderSurface pointer
    MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
    MY3D_PARAM(P_CAMERA, Camera);                // Camera pointer
}

/// A view has allocated its screen buffers for rendering. They can be accessed now with View::FindNamedTexture().
MY3D_EVENT(E_VIEWBUFFERSREADY, ViewBuffersReady)
{
    MY3D_PARAM(P_VIEW, View);                    // View pointer
    MY3D_PARAM(P_TEXTURE, Texture);              // Texture pointer
    MY3D_PARAM(P_SURFACE, Surface);              // RenderSurface pointer
    MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
    MY3D_PARAM(P_CAMERA, Camera);                // Camera pointer
}

/// A view has set global shader parameters for a new combination of vertex/pixel shaders. Custom global parameters can now be set.
MY3D_EVENT(E_VIEWGLOBALSHADERPARAMETERS, ViewGlobalShaderParameters)
{
    MY3D_PARAM(P_VIEW, View);                    // View pointer
    MY3D_PARAM(P_TEXTURE, Texture);              // Texture pointer
    MY3D_PARAM(P_SURFACE, Surface);              // RenderSurface pointer
    MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
    MY3D_PARAM(P_CAMERA, Camera);                // Camera pointer
}

/// Render of a view ended. Its screen buffers are still accessible if needed.
MY3D_EVENT(E_ENDVIEWRENDER, EndViewRender)
{
    MY3D_PARAM(P_VIEW, View);                    // View pointer
    MY3D_PARAM(P_TEXTURE, Texture);              // Texture pointer
    MY3D_PARAM(P_SURFACE, Surface);              // RenderSurface pointer
    MY3D_PARAM(P_SCENE, Scene);                  // Scene pointer
    MY3D_PARAM(P_CAMERA, Camera);                // Camera pointer
}

}
