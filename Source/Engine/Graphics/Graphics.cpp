//
// Created by luchu on 2022/1/18.
//

#include "SDL.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsEvents.h"
#include "Core/Context.h"
#include "IO/Log.h"


namespace My3D
{

void Graphics::SetExternalWindow(void* window)
{
    if (!window_)
        externalWindow_ = window;
    else
        MY3D_LOGERROR("Window already opened, can not set external window");
}

void Graphics::AdjustScreenMode(int& newWidth, int& newHeight, ScreenModeParams& params, bool& maximize) const
{
    const int numMonitors = SDL_GetNumVideoDisplays();
    if (params.monitor_ >= numMonitors || params.monitor_ < 0)
        params.monitor_ = 0;

    if (params.fullscreen_ || params.borderless_)
    {
        params.resizable_ = false;
        maximize = false;
    }

    // Borderless cannot be fullscreen
    if (params.borderless_)
        params.fullscreen_ = false;

    // Ensure that multisample factor is in valid range
    params.multiSample_ = NextPowerOfTwo(Clamp(params.multiSample_, 1, 16));

    if (!newWidth || !newHeight)
    {
        if (params.fullscreen_ || params.borderless_)
        {
            SDL_DisplayMode mode;
            SDL_GetDesktopDisplayMode(params.monitor_, &mode);
            newWidth = mode.w;
            newHeight = mode.h;
        }
        else
        {
            newWidth = 800;
            newHeight = 600;
        }
    }

    if (params.fullscreen_)
    {
        const PODVector<IntVector3> resolution = GetResolutions(params.monitor_);
        if (!resolution.Empty())
        {
            const unsigned bestResolution = FindBestResolutionIndex(params.monitor_, newWidth, newHeight, params.refreshRate_);
            newWidth = resolution[bestResolution].x_;
            newHeight = resolution[bestResolution].y_;
            params.refreshRate_ = resolution[bestResolution].z_;
        }
    }
    else
    {
        // if windowed, use the same refresh rate as desktop
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(params.monitor_, &mode);
        params.refreshRate_ = mode.refresh_rate;
    }
}

void Graphics::SetWindowTitle(const String &windowTitle)
{
    windowTitle_ = windowTitle;
    if (window_)
        SDL_SetWindowTitle(window_, windowTitle_.CString());
}

void Graphics::SetWindowPosition(const IntVector2& position)
{
    if (window_)
        SDL_SetWindowPosition(window_, position.x_, position.y_);
    else
        position_ = position;
}

bool Graphics::SetScreenMode(int width, int height)
{
    return SetScreenMode(width, height, screenParams_);
}

void Graphics::SetWindowPosition(int x, int y)
{
    SetWindowPosition(IntVector2(x, y));
}

void Graphics::Maximize()
{
    if (!window_)
        return;

    SDL_MaximizeWindow(window_);
}

void Graphics::Minimize()
{
    if (!window_)
        return;

    SDL_MinimizeWindow(window_);
}

void Graphics::Raise() const
{
    if (!window_)
        return;
    SDL_RaiseWindow(window_);
}

void Graphics::OnScreenModeChanged()
{
#ifdef MY3D_LOGGING
    String msg;
    msg.AppendWithFormat("Set screen mode %dx%d rate %d Hz %s monitor %d", width_, height_, screenParams_.refreshRate_,
    (screenParams_.fullscreen_ ? "fullscreen" : "windowed"), screenParams_.monitor_);
    if (screenParams_.borderless_)
    msg.Append(" borderless");
    if (screenParams_.resizable_)
    msg.Append(" resizable");
    if (screenParams_.multiSample_ > 1)
    msg.AppendWithFormat(" multisample %d", screenParams_.multiSample_);
    MY3D_LOGINFO(msg);
#endif

    using namespace ScreenMode;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_WIDTH] = width_;
    eventData[P_HEIGHT] = height_;
    eventData[P_FULLSCREEN] = screenParams_.fullscreen_;
    eventData[P_BORDERLESS] = screenParams_.borderless_;
    eventData[P_RESIZABLE] = screenParams_.resizable_;
    eventData[P_MONITOR] = screenParams_.monitor_;
    eventData[P_REFRESHRATE] = screenParams_.refreshRate_;
    SendEvent(E_SCREENMODE, eventData);
}

bool Graphics::ToggleFullscreen()
{
    Swap(primaryWindowMode_, secondaryWindowMode_);
    return SetScreenMode(primaryWindowMode_.width_, primaryWindowMode_.height_, primaryWindowMode_.screenParams_);
}

PODVector<IntVector3> Graphics::GetResolutions(int monitor) const
{
    PODVector<IntVector3> ret;
    auto numModes = (unsigned) SDL_GetNumDisplayModes(monitor);
    for (unsigned i = 0; i < numModes; ++i)
    {
        SDL_DisplayMode mode;
        SDL_GetDisplayMode(monitor, i, &mode);
        int width = mode.w;
        int height = mode.h;
        int rate = mode.refresh_rate;

        bool unique = true;
        for (unsigned j = 0; j < ret.Size(); ++j)
        {
            if (ret[j].x_ == width && ret[j].y_ == height && ret[j].z_ == rate)
            {
                unique = false;
                break;
            }
        }

        if (unique)
            ret.Push(IntVector3(width, height, rate));
    }

    return ret;
}

unsigned Graphics::FindBestResolutionIndex(int monitor, int width, int height, int refreshRate) const
{
    const PODVector<IntVector3> resolutions = GetResolutions(monitor);
    if (resolutions.Empty())
        return M_MAX_UNSIGNED;

    unsigned best = 0;
    unsigned bestError = M_MAX_UNSIGNED;

    for (unsigned i = 0; i < resolutions.Size(); ++i)
    {
        auto error = static_cast<unsigned>(Abs(resolutions[i].x_ - width) + Abs(resolutions[i].y_ - height));
        if (refreshRate != 0)
            error += static_cast<unsigned>(Abs(resolutions[i].z_ - refreshRate));
        if (error < bestError)
        {
            best = i;
            bestError = error;
        }
    }

    return best;
}

void RegisterGraphicsLibrary(Context* context)
{

}

}

