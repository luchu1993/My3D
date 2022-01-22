//
// Created by luchu on 2022/1/18.
//

#include "SDL.h"
#include "SDL_syswm.h"
#include "Core/Context.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/VertexBuffer.h"
#include "IO/Log.h"


namespace My3D
{
    static HWND GetWindowHandle(SDL_Window* window)
    {
        SDL_SysWMinfo sysInfo;

        SDL_VERSION(&sysInfo.version);
        SDL_GetWindowWMInfo(window, &sysInfo);

        return sysInfo.info.win.window;
    }

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

    IntVector2 Graphics::GetRenderTargetDimensions() const
    {
        int width, height;

        if (renderTargets_[0])
        {
            width = renderTargets_[0]->GetWidth();
        }
        return IntVector2(width, height);
    }

    void Graphics::Clear(ClearTargetFlags flags, const Color &color, float depth, unsigned int stencil)
    {
        IntVector2 rtSize = GetRenderTargetDimensions();
    }

    void Graphics::SetVertexBuffer(VertexBuffer *buffer)
    {
        static PODVector<VertexBuffer*> vertexBuffers(1);
        vertexBuffers[0] = buffer;
        SetVertexBuffers(vertexBuffers);
    }

    bool Graphics::SetVertexBuffers(const PODVector<VertexBuffer*> &buffers, unsigned int instanceOffset)
    {
        if (buffers.Size() > MAX_VERTEX_STREAMS)
        {
            MY3D_LOGERROR("Too many vertex buffers");
            return false;
        }

        for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
        {
            VertexBuffer* buffer = nullptr;
            bool changed = false;

            buffer = i < buffers.Size() ? buffers[i] : nullptr;
            if (buffer)
            {
                const PODVector<VertexElement>& elements = buffer->GetElements();
                // Check if buffer has per-instance data
                bool hasInstanceData = elements.Size() && elements[0].perInstance_;
                unsigned offset = hasInstanceData ? instanceOffset * buffer->GetVertexSize() : 0;

                if (buffer != vertexBuffers_[i] || offset != impl_->vertexOffsets_[i])
                {
                    vertexBuffers_[i] = buffer;
                    impl_->vertexBuffers_[i] = (ID3D11Buffer*) buffer->GetGPUObject();
                    impl_->vertexSizes_[i] = buffer->GetVertexSize();
                    impl_->vertexOffsets_[i] = offset;
                    changed = true;
                }
            }
            else if (vertexBuffers_[i])
            {
                vertexBuffers_[i] = nullptr;
                impl_->vertexBuffers_[i] = nullptr;
                impl_->vertexSizes_[i] = 0;
                impl_->vertexOffsets_[i] = 0;
                changed = true;
            }

            if (changed)
            {
                impl_->vertexDeclarationDirty_ = true;
                if (impl_->firstDirtyVB_ == M_MAX_UNSIGNED)
                    impl_->firstDirtyVB_ = impl_->lastDirtyVB_ = i;
                else
                {
                    if (i < impl_->firstDirtyVB_)
                        impl_->firstDirtyVB_ = i;
                    if (i > impl_->lastDirtyVB_)
                        impl_->lastDirtyVB_ = i;
                }
            }
        }

        return true;
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
            CreateDevice(width, height);
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

        // Check that multi-sample level is supported
        PODVector<int> multiSampleLevels = GetMultiSampleLevels();
        if (!multiSampleLevels.Contains(screenParams_.multiSample_))
            screenParams_.multiSample_ = 1;

        // Create swap chain. Release old if necessary
        if (impl_->swapChain_)
        {
            impl_->swapChain_->Release();
            impl_->swapChain_ = nullptr;
        }

        IDXGIDevice* dxgiDevice = nullptr;
        impl_->device_->QueryInterface(IID_IDXGIDevice, (void**)&dxgiDevice);
        IDXGIAdapter* dxgiAdapter = nullptr;
        dxgiDevice->GetParent(IID_IDXGIAdapter, (void**)&dxgiAdapter);
        IDXGIFactory* dxgiFactory = nullptr;
        dxgiAdapter->GetParent(IID_IDXGIFactory, (void**)&dxgiFactory);

        DXGI_RATIONAL refreshRateRational = {};
        IDXGIOutput* dxgiOutput = nullptr;
        UINT numModes = 0;
        dxgiAdapter->EnumOutputs(screenParams_.monitor_, &dxgiOutput);
        dxgiOutput->GetDisplayModeList(sRGB_ ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, nullptr);

        // Find the best matching refresh rate with the specified resolution
        if (numModes > 0)
        {
            DXGI_MODE_DESC* modes = new DXGI_MODE_DESC[numModes];
            dxgiOutput->GetDisplayModeList(sRGB_ ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numModes, modes);
            unsigned bestMatchingRateIndex = -1;
            unsigned bestError = M_MAX_UNSIGNED;
            for (unsigned i = 0; i < numModes; ++i)
            {
                if (width != modes[i].Width || height != modes[i].Height)
                    continue;

                float rate = (float) modes[i].RefreshRate.Numerator / modes[i].RefreshRate.Denominator;
                unsigned error = (unsigned)(Abs(rate - screenParams_.refreshRate_));
                if (error < bestError)
                {
                    bestMatchingRateIndex = i;
                    bestError = error;
                }
            }

            if (bestMatchingRateIndex != -1)
            {
                refreshRateRational.Numerator = modes[bestMatchingRateIndex].RefreshRate.Numerator;
                refreshRateRational.Denominator = modes[bestMatchingRateIndex].RefreshRate.Denominator;
            }
            delete[] modes;
        }

        dxgiOutput->Release();

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        memset(&swapChainDesc, 0, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = (UINT) width;
        swapChainDesc.BufferDesc.Height = (UINT) height;
        swapChainDesc.BufferDesc.Format = sRGB_ ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = refreshRateRational.Numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = refreshRateRational.Denominator;
        swapChainDesc.OutputWindow = GetWindowHandle(window_);
        swapChainDesc.SampleDesc.Count = static_cast<UINT>(screenParams_.multiSample_);
        swapChainDesc.SampleDesc.Quality = impl_->GetMultiSampleQuality(swapChainDesc.BufferDesc.Format, screenParams_.multiSample_);
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        HRESULT hr = dxgiFactory->CreateSwapChain(impl_->device_, &swapChainDesc, &impl_->swapChain_);
        // After creating the swap chain, disable automatic Alt-Enter fullscreen/windowed switching
        // (the application will switch manually if it wants to)
        dxgiFactory->MakeWindowAssociation(GetWindowHandle(window_), DXGI_MWA_NO_ALT_ENTER);

#if MY3D_LOGGING
        DXGI_ADAPTER_DESC desc;
        dxgiAdapter->GetDesc(&desc);
        String adapterDesc(desc.Description);
        MY3D_LOGINFO("Adapter used " + adapterDesc);
#endif

        dxgiFactory->Release();
        dxgiAdapter->Release();
        dxgiDevice->Release();

        if (FAILED(hr))
        {
            MY3D_SAFE_RELEASE(impl_->swapChain_);
            MY3D_LOGD3DERROR("Failed to created D3D11 swap chain", hr);
            return false;
        }

        return true;
    }

    bool Graphics::UpdateSwapChain(int width, int height)
    {
        bool success = true;

        ID3D11RenderTargetView* nullView = nullptr;
        impl_->deviceContext_->OMSetRenderTargets(1, &nullView, nullptr);
        if (impl_->defaultRenderTargetView_)
        {
            impl_->defaultRenderTargetView_->Release();
            impl_->defaultRenderTargetView_ = nullptr;
        }

        if (impl_->defaultDepthStencilView_)
        {
            impl_->defaultDepthStencilView_->Release();
            impl_->defaultDepthStencilView_ = nullptr;
        }

        if (impl_->defaultDepthTexture_)
        {
            impl_->defaultDepthTexture_->Release();
            impl_->defaultDepthTexture_ = nullptr;
        }

        impl_->depthStencilView = nullptr;
        for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
            impl_->renderTargetViews_[i] = nullptr;
        impl_->renderTargetsDirty_ = true;

        impl_->swapChain_->ResizeBuffers(1, (UINT) width, (UINT) height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

        // Create default render target view representing backbuffer
        ID3D11Texture2D* backbufferTexture;
        HRESULT hr = impl_->swapChain_->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbufferTexture);
        if (FAILED(hr))
        {
            MY3D_SAFE_RELEASE(backbufferTexture);
            MY3D_LOGD3DERROR("Failed to get backbuffer texture", hr);
            success = false;
        }
        else
        {
            hr = impl_->device_->CreateRenderTargetView(backbufferTexture, nullptr, &impl_->defaultRenderTargetView_);
            backbufferTexture->Release();
            if (FAILED(hr))
            {
                MY3D_SAFE_RELEASE(impl_->defaultRenderTargetView_);
                MY3D_LOGD3DERROR("Failed to create backbuffer render target view", hr);
                success = false;
            }
        }

        // Create default depth-stencil texture and view
        D3D11_TEXTURE2D_DESC depthDesc;
        memset(&depthDesc, 0, sizeof depthDesc);
        depthDesc.Width = (UINT) width;
        depthDesc.Height = (UINT) height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = static_cast<UINT>(screenParams_.multiSample_);
        depthDesc.SampleDesc.Quality = impl_->GetMultiSampleQuality(depthDesc.Format, screenParams_.multiSample_);
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDesc.CPUAccessFlags = 0;
        depthDesc.MiscFlags = 0;
        hr = impl_->device_->CreateTexture2D(&depthDesc, nullptr, &impl_->defaultDepthTexture_);
        if (FAILED(hr))
        {
            MY3D_SAFE_RELEASE(impl_->defaultDepthTexture_);
            MY3D_LOGD3DERROR("Failed to create backbuffer depth-stencil texture", hr);
            success = false;
        }
        else
        {
            hr = impl_->device_->CreateDepthStencilView(impl_->defaultDepthTexture_, nullptr, &impl_->defaultDepthStencilView_);
            if (FAILED(hr))
            {
                MY3D_SAFE_RELEASE(impl_->defaultDepthStencilView_);
                MY3D_LOGD3DERROR("Failed to create backbuffer depth-stencil view", hr);
                success = false;
            }
        }

        // Update internally held backbuffer size
        width_ = width;
        height_ = height;

        ResetRenderTargets();
        return success;
    }

    void Graphics::ResetRenderTargets()
    {

    }

    void Graphics::ResetRenderTarget(unsigned int index)
    {

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
                if (impl_->CheckMultiSampleSupport(sRGB_ ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, i))
                    ret.Push(i);
            }
        }

        return ret;
    }

    void Graphics::CheckFeatureSupport()
    {

    }

    bool Graphics::IsInitialized() const
    {
        return window_ != nullptr && impl_->GetDevice() != nullptr;
    }

    VertexBuffer *Graphics::GetVertexBuffer(unsigned int index) const
    {
        return index < MAX_VERTEX_STREAMS ? vertexBuffers_[index] : nullptr;
    }
}
