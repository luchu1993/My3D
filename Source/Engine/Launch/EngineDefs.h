//
// Created by luchu on 2022/1/13.
//

#pragma once

#include "Container/String.h"


namespace My3D
{
    // Log
    static const String EP_LOG_LEVEL = "LogLevel";
    static const String EP_LOG_NAME = "LogName";
    static const String EP_LOG_QUIET = "LogQuiet";

    // Window
    static const String EP_WINDOW_TITLE = "WindowTitle";
    static const String EP_HEADLESS = "Headless";
    static const String EP_WINDOW_WIDTH = "WindowWidth";
    static const String EP_WINDOW_HEIGHT = "WindowHeight";
    static const String EP_WINDOW_POSITION_X = "WindowPositionX";
    static const String EP_WINDOW_POSITION_Y = "WindowPositionY";
    static const String EP_WINDOW_RESIZABLE = "WindowResizable";

    // Screen
    static const String EP_FULL_SCREEN = "FullScreen";
    static const String EP_BORDERLESS = "Borderless";
    static const String EP_HIGH_DPI = "HighDPI";
    static const String EP_TRIPLE_BUFFER = "TripleBuffer";
    static const String EP_VSYNC = "VSync";
    static const String EP_MONITOR = "Monitor";
    static const String EP_MULTI_SAMPLE = "MultiSample";
    static const String EP_REFRESH_RATE = "RefreshRate";

    // Input
    static const String EP_TOUCH_EMULATION = "TouchEmulation";

    // Frame Limit
    static const String EP_FRAME_LIMITER = "FrameLimiter";
}

