//
// Created by luchu on 2022/1/18.
//

#include "SDL.h"
#include "SDL_syswm.h"
#include "Core/Context.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "IO/Log.h"


namespace My3D
{
    Graphics::Graphics(Context *context)
        : Base(context)
        , impl_(new GraphicsImpl())
        , position_(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED)
        , orientations_("LandscapeLeft LandscapeRight")
        , apiName_("D3D11")
    {

        context_->RequireSDL(SDL_INIT_VIDEO);
        // Register Graphics library object factories
        RegisterGraphicsLibrary(context_);
    }

    Graphics::~Graphics()
    {
        if (window_)
        {
            SDL_ShowCursor(SDL_TRUE);
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }

        delete impl_;
        impl_ = nullptr;

        context_->ReleaseSDL();
    }

    void Graphics::Close()
    {
        if (window_)
        {
            SDL_ShowCursor(SDL_TRUE);
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
    }

    bool Graphics::SetScreenMode(int width, int height, const ScreenModeParams& params, bool maximize)
    {
        // Ensure that parameters are properly filled.
        ScreenModeParams newParams = params;
        AdjustScreenMode(width, height, newParams, maximize);

        // Find out the full screen mode display format (match desktop color depth)
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(newParams.monitor_, &mode);
        const DXGI_FORMAT fullscreenFormat = SDL_BITSPERPIXEL(mode.format) == 16 ? DXGI_FORMAT_B5G6R5_UNORM : DXGI_FORMAT_B8G8R8A8_UNORM;

        if (width == width_ && height == height_ && newParams == screenParams_)
            return true;

        SDL_SetHint(SDL_HINT_ORIENTATIONS, orientations_.CString());
        if (!window_)
        {
            if (!OpenWindow(width, height, newParams.resizable_, newParams.borderless_))
                return false;
        }

        AdjustWindow(width, height, newParams.fullscreen_, newParams.borderless_, newParams.monitor_);

        if (maximize)
        {
            Maximize();
            SDL_GetWindowSize(window_, &width, &height);
        }

        const int oldMultiSample = screenParams_.multiSample_;
        screenParams_ = newParams;

        if (!impl_->device_ || screenParams_.multiSample_ != oldMultiSample)
            CreateDevice();
        UpdateSwapChain(width, height);

        // Clear the initial window contents to black
        Clear(CLEAR_COLOR);
        impl_->swapChain_->Present(0, 0);

        OnScreenModeChanged();
        return true;
    }

    bool Graphics::OpenWindow(int width, int height, bool resizable, bool borderless)
    {
        unsigned flags = 0;
        if (resizable)
            flags |= SDL_WINDOW_RESIZABLE;
        if (borderless)
            flags |= SDL_WINDOW_BORDERLESS;

        window_ = SDL_CreateWindow(windowTitle_.CString(), position_.x_, position_.y_, width, height, flags);
        if (!window_)
        {
            MY3D_LOGERRORF("Could not create window. root cause: '%s'", SDL_GetError());
            return false;
        }

        SDL_GetWindowPosition(window_, &position_.x_, &position_.y_);

        return true;
    }

    void Graphics::AdjustWindow(int &newWidth, int &newHeight, bool &newFullscreen, bool &newBorderless, int &monitor)
    {
        if (!externalWindow_)
        {

        }
    }

    bool Graphics::CreateDevice(int width, int height)
    {
        // Device needs only to be created once
        if (!impl_->device_)
        {
            HRESULT hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                0,
                nullptr,
                0,
                D3D11_SDK_VERSION,
                &impl_->device_,
                nullptr,
                &impl_->deviceContext_
            );
            if (FAILED(hr))
            {
                MY3D_SAFE_RELEASE(impl_->device_);
                MY3D_SAFE_RELEASE(impl_->deviceContext_);
                MY3D_LOGD3DERROR("Failed to create D3D11 device", hr);
                return false;
            }

            CheckFeatureSupport();
            // Set the flush mode
            SetFlushGPU(flushGPU_);
        }

        PODVector<int> multiSampleLevels = GetMultiSampleLevls();
    }

    void Graphics::SetFlushGPU(bool enable)
    {
        flushGPU_ = enable;
        if (impl_->device_)
        {
            IDXGIDevice1* dxgiDevice;
            impl_->device_->QueryInterface(IID_IDXGIDevice1, (void**)(&dxgiDevice));
            if (dxgiDevice)
            {
                dxgiDevice->SetMaximumFrameLatency(enable ? 1 : 3);
                dxgiDevice->Release();
            }
        }
    }

    PODVector<int> Graphics::GetMultiSampleLevels() const
    {
        PODVector<int> ret;
        ret.Push(1);

        if (impl_->device_)
        {
            for (unsigned i = 2; i <= 16; ++i)
            {
                if (impl_->CheckMultiSampleSupport())
            }
        }
    }

    void Graphics::CheckFeatureSupport()
    {

    }

    bool Graphics::IsInitialized() const
    {
        return window_ != nullptr && impl_->GetDevice() != nullptr;
    }
}
