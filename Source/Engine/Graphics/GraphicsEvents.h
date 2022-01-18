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

/// Frame rendering started.
MY3D_EVENT(E_BEGINRENDERING, BeginRendering)
{
}

/// Frame rendering ended.
MY3D_EVENT(E_ENDRENDERING, EndRendering)
{
}

}
