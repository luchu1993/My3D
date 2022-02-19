//
// Created by luchu on 2022/1/18.
//

#include <SDL.h>
#include <SDL_syswm.h>

#include "Core/Context.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/ShaderPrecache.h"
#include "IO/Log.h"


// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
__declspec(dllexport) DWORD NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace My3D
{
    static const D3D11_COMPARISON_FUNC d3dCmpFunc[] =
    {
        D3D11_COMPARISON_ALWAYS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_GREATER_EQUAL
    };

    static const DWORD d3dBlendEnable[] =
    {
        FALSE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        TRUE,
        TRUE
    };

    static const D3D11_BLEND d3dSrcBlend[] =
    {
        D3D11_BLEND_ONE,
        D3D11_BLEND_ONE,
        D3D11_BLEND_DEST_COLOR,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_ONE,
        D3D11_BLEND_INV_DEST_ALPHA,
        D3D11_BLEND_ONE,
        D3D11_BLEND_SRC_ALPHA,
    };

    static const D3D11_BLEND d3dDestBlend[] =
    {
        D3D11_BLEND_ZERO,
        D3D11_BLEND_ONE,
        D3D11_BLEND_ZERO,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_ONE,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_ONE,
        D3D11_BLEND_ONE
    };

    static const D3D11_BLEND_OP d3dBlendOp[] =
    {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT
    };

    static const D3D11_STENCIL_OP d3dStencilOp[] =
    {
        D3D11_STENCIL_OP_KEEP,
        D3D11_STENCIL_OP_ZERO,
        D3D11_STENCIL_OP_REPLACE,
        D3D11_STENCIL_OP_INCR,
        D3D11_STENCIL_OP_DECR
    };

    static const D3D11_CULL_MODE d3dCullMode[] =
    {
        D3D11_CULL_NONE,
        D3D11_CULL_BACK,
        D3D11_CULL_FRONT
    };

    static const D3D11_FILL_MODE d3dFillMode[] =
    {
        D3D11_FILL_SOLID,
        D3D11_FILL_WIREFRAME,
        D3D11_FILL_WIREFRAME // Point fill mode not supported
    };

    static void GetD3DPrimitiveType(unsigned elementCount, PrimitiveType type, unsigned& primitiveCount, D3D_PRIMITIVE_TOPOLOGY& d3dPrimitiveType)
    {
        switch (type)
        {
            case TRIANGLE_LIST:
                primitiveCount = elementCount / 3;
                d3dPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;

            case LINE_LIST:
                primitiveCount = elementCount / 2;
                d3dPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                break;

            case POINT_LIST:
                primitiveCount = elementCount;
                d3dPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;

            case TRIANGLE_STRIP:
                primitiveCount = elementCount - 2;
                d3dPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;

            case LINE_STRIP:
                primitiveCount = elementCount - 1;
                d3dPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;

            case TRIANGLE_FAN:
                // Triangle fan is not supported on D3D11
                primitiveCount = 0;
                d3dPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                break;
        }
    }

    static HWND GetWindowHandle(SDL_Window* window)
    {
        SDL_SysWMinfo sysInfo;

        SDL_VERSION(&sysInfo.version);
        SDL_GetWindowWMInfo(window, &sysInfo);

        return sysInfo.info.win.window;
    }

    const Vector2 Graphics::pixelUVOffset(0.0f, 0.0f);
    bool Graphics::gl3Support = false;

    Graphics::Graphics(Context *context)
        : Base(context)
        , impl_(new GraphicsImpl())
        , position_(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED)
        , orientations_("LandscapeLeft LandscapeRight")
        , apiName_("D3D11")
    {

        SetTextureUnitMappings();
        ResetCachedState();

        context_->RequireSDL(SDL_INIT_VIDEO);

        // Register Graphics library object factories
        RegisterGraphicsLibrary(context_);
    }

    Graphics::~Graphics()
    {
        {
            MutexLock lock(gpuObjectMutex_);
            for (auto const& gpuObject : gpuObjects_)
                gpuObject->Release();

            gpuObjects_.Clear();
        }

        MY3D_SAFE_RELEASE(impl_->defaultRenderTargetView_);
        MY3D_SAFE_RELEASE(impl_->defaultDepthStencilView_);
        MY3D_SAFE_RELEASE(impl_->defaultDepthTexture_);
        MY3D_SAFE_RELEASE(impl_->resolveTexture_);
        MY3D_SAFE_RELEASE(impl_->swapChain_);
        MY3D_SAFE_RELEASE(impl_->deviceContext_);
        MY3D_SAFE_RELEASE(impl_->device_);

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

    bool Graphics::SetScreenMode(int width, int height, const ScreenModeParams& params, bool maximize)
    {
        // Ensure that parameters are properly filled.
        ScreenModeParams newParams = params;
        AdjustScreenMode(width, height, newParams, maximize);

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

    IntVector2 Graphics::GetRenderTargetDimensions() const
    {
        int width, height;

        if (renderTargets_[0])
        {
            width = renderTargets_[0]->GetWidth();
            height = renderTargets_[0]->GetHeight();
        }
        else if (depthStencil_)
        {
            width = depthStencil_->GetWidth();
            height = depthStencil_->GetHeight();
        }
        else
        {
            width = width_;
            height = height_;
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

    bool Graphics::ResolveToTexture(Texture2D* destination, const IntRect& viewport)
    {
        if (!destination || !destination->GetRenderSurface())
            return false;

        IntRect vpCopy = viewport;
        if (vpCopy.right_ <= vpCopy.left_)
            vpCopy.right_ = vpCopy.left_ + 1;
        if (vpCopy.bottom_ <= vpCopy.top_)
            vpCopy.bottom_ = vpCopy.top_ + 1;

        D3D11_BOX srcBox;
        srcBox.left = Clamp(vpCopy.left_, 0, width_);
        srcBox.top = Clamp(vpCopy.top_, 0, height_);
        srcBox.right = Clamp(vpCopy.right_, 0, width_);
        srcBox.bottom = Clamp(vpCopy.bottom_, 0, height_);
        srcBox.front = 0;
        srcBox.back = 1;

        ID3D11Resource* source = nullptr;
        const bool resolve = screenParams_.multiSample_ > 1;
        impl_->defaultRenderTargetView_->GetResource(&source);

        if (!resolve)
        {
            if (!srcBox.left && !srcBox.top && srcBox.right == width_ && srcBox.bottom == height_)
                impl_->deviceContext_->CopyResource((ID3D11Resource*)destination->GetGPUObject(), source);
            else
                impl_->deviceContext_->CopySubresourceRegion((ID3D11Resource*)destination->GetGPUObject(), 0, 0, 0, 0, source, 0, &srcBox);
        }
        else
        {
            if (!srcBox.left && !srcBox.top && srcBox.right == width_ && srcBox.bottom == height_)
            {
                impl_->deviceContext_->ResolveSubresource((ID3D11Resource*)destination->GetGPUObject(), 0, source, 0, (DXGI_FORMAT)
                        destination->GetFormat());
            }
            else
            {
                CreateResolveTexture();

                if (impl_->resolveTexture_)
                {
                    impl_->deviceContext_->ResolveSubresource(impl_->resolveTexture_, 0, source, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
                    impl_->deviceContext_->CopySubresourceRegion((ID3D11Resource*)destination->GetGPUObject(), 0, 0, 0, 0, impl_->resolveTexture_, 0, &srcBox);
                }
            }
        }

        source->Release();

        return true;
    }

    bool Graphics::ResolveToTexture(Texture2D* texture)
    {
        if (!texture)
            return false;
        RenderSurface* surface = texture->GetRenderSurface();
        if (!surface)
            return false;

        texture->SetResolveDirty(false);
        surface->SetResolveDirty(false);
        ID3D11Resource* source = (ID3D11Resource*)texture->GetGPUObject();
        ID3D11Resource* dest = (ID3D11Resource*)texture->GetResolveTexture();
        if (!source || !dest)
            return false;

        impl_->deviceContext_->ResolveSubresource(dest, 0, source, 0, (DXGI_FORMAT)texture->GetFormat());
        return true;
    }

    bool Graphics::ResolveToTexture(TextureCube* texture)
    {
        if (!texture)
            return false;

        texture->SetResolveDirty(false);
        ID3D11Resource* source = (ID3D11Resource*)texture->GetGPUObject();
        ID3D11Resource* dest = (ID3D11Resource*)texture->GetResolveTexture();
        if (!source || !dest)
            return false;

        for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
        {
            // Resolve only the surface(s) that were actually rendered to
            RenderSurface* surface = texture->GetRenderSurface((CubeMapFace)i);
            if (!surface->IsResolveDirty())
                continue;

            surface->SetResolveDirty(false);
            unsigned subResource = D3D11CalcSubresource(0, i, texture->GetLevels());
            impl_->deviceContext_->ResolveSubresource(dest, subResource, source, subResource, (DXGI_FORMAT)texture->GetFormat());
        }

        return true;
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

    bool Graphics::SetVertexBuffers(const Vector<SharedPtr<VertexBuffer> >& buffers, unsigned instanceOffset)
    {
        return SetVertexBuffers(reinterpret_cast<const PODVector<VertexBuffer*>&>(buffers), instanceOffset);
    }

    void Graphics::SetIndexBuffer(IndexBuffer* buffer)
    {
        if (buffer != indexBuffer_)
        {
            if (buffer)
                impl_->deviceContext_->IASetIndexBuffer((ID3D11Buffer*)buffer->GetGPUObject(),
                                                        buffer->GetIndexSize() == sizeof(unsigned short) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
            else
                impl_->deviceContext_->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);

            indexBuffer_ = buffer;
        }
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

    unsigned Graphics::GetAlphaFormat()
    {
        return DXGI_FORMAT_A8_UNORM;
    }

    unsigned Graphics::GetLuminanceFormat()
    {
        // Note: not same sampling behavior as on D3D9; need to sample the R channel only
        return DXGI_FORMAT_R8_UNORM;
    }

    unsigned Graphics::GetLuminanceAlphaFormat()
    {
        // Note: not same sampling behavior as on D3D9; need to sample the RG channels
        return DXGI_FORMAT_R8G8_UNORM;
    }

    unsigned Graphics::GetRGBFormat()
    {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    unsigned Graphics::GetRGBAFormat()
    {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    unsigned Graphics::GetRGBA16Format()
    {
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    }

    unsigned Graphics::GetRGBAFloat16Format()
    {
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    }

    unsigned Graphics::GetRGBAFloat32Format()
    {
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    }

    unsigned Graphics::GetRG16Format()
    {
        return DXGI_FORMAT_R16G16_UNORM;
    }

    unsigned Graphics::GetRGFloat16Format()
    {
        return DXGI_FORMAT_R16G16_FLOAT;
    }

    unsigned Graphics::GetRGFloat32Format()
    {
        return DXGI_FORMAT_R32G32_FLOAT;
    }

    unsigned Graphics::GetFloat16Format()
    {
        return DXGI_FORMAT_R16_FLOAT;
    }

    unsigned Graphics::GetFloat32Format()
    {
        return DXGI_FORMAT_R32_FLOAT;
    }

    unsigned Graphics::GetLinearDepthFormat()
    {
        return DXGI_FORMAT_R32_FLOAT;
    }

    unsigned Graphics::GetDepthStencilFormat()
    {
        return DXGI_FORMAT_R24G8_TYPELESS;
    }

    unsigned Graphics::GetReadableDepthFormat()
    {
        return DXGI_FORMAT_R24G8_TYPELESS;
    }

    unsigned Graphics::GetFormat(const String& formatName)
    {
        String nameLower = formatName.ToLower().Trimmed();

        if (nameLower == "a")
            return GetAlphaFormat();
        if (nameLower == "l")
            return GetLuminanceFormat();
        if (nameLower == "la")
            return GetLuminanceAlphaFormat();
        if (nameLower == "rgb")
            return GetRGBFormat();
        if (nameLower == "rgba")
            return GetRGBAFormat();
        if (nameLower == "rgba16")
            return GetRGBA16Format();
        if (nameLower == "rgba16f")
            return GetRGBAFloat16Format();
        if (nameLower == "rgba32f")
            return GetRGBAFloat32Format();
        if (nameLower == "rg16")
            return GetRG16Format();
        if (nameLower == "rg16f")
            return GetRGFloat16Format();
        if (nameLower == "rg32f")
            return GetRGFloat32Format();
        if (nameLower == "r16f")
            return GetFloat16Format();
        if (nameLower == "r32f" || nameLower == "float")
            return GetFloat32Format();
        if (nameLower == "lineardepth" || nameLower == "depth")
            return GetLinearDepthFormat();
        if (nameLower == "d24s8")
            return GetDepthStencilFormat();
        if (nameLower == "readabledepth" || nameLower == "hwdepth")
            return GetReadableDepthFormat();

        return GetRGBFormat();
    }

    unsigned Graphics::GetMaxBones()
    {
        return 128;
    }

    bool Graphics::GetGL3Support()
    {
        return gl3Support;
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
            // Keep current window position because it may change in intermediate callbacks
            const IntVector2 oldPosition = position_;
            bool reposition = false;
            bool resizePostponed = false;
            if (!newWidth || !newHeight)
            {
                SDL_MaximizeWindow(window_);
                SDL_GetWindowSize(window_, &newWidth, &newHeight);
            }
            else
            {
                SDL_Rect display_rect;
                SDL_GetDisplayBounds(monitor, &display_rect);

                reposition = newFullscreen || (newBorderless && newWidth >= display_rect.w && newHeight >= display_rect.h);
                if (reposition)
                {
                    // Reposition the window on the specified monitor if it's supposed to cover the entire monitor
                    SDL_SetWindowPosition(window_, display_rect.x, display_rect.y);
                }

                // Postpone window resize if exiting fullscreen to avoid redundant resolution change
                if (!newFullscreen && screenParams_.fullscreen_)
                    resizePostponed = true;
                else
                    SDL_SetWindowSize(window_, newWidth, newHeight);
            }

            // Turn off window fullscreen mode so it gets repositioned to the correct monitor
            SDL_SetWindowFullscreen(window_, SDL_FALSE);
            // Hack fix: on SDL 2.0.4 a fullscreen->windowed transition results in a maximized window when the D3D device is reset, so hide before
            if (!newFullscreen) SDL_HideWindow(window_);
            SDL_SetWindowFullscreen(window_, newFullscreen ? SDL_WINDOW_FULLSCREEN : 0);
            SDL_SetWindowBordered(window_, newBorderless ? SDL_FALSE : SDL_TRUE);
            if (!newFullscreen) SDL_ShowWindow(window_);

            // Resize now if was postponed
            if (resizePostponed)
                SDL_SetWindowSize(window_, newWidth, newHeight);

            // Ensure that window keeps its position
            if (!reposition)
                SDL_SetWindowPosition(window_, oldPosition.x_, oldPosition.y_);
            else
                position_ = oldPosition;
        }
        else
        {
            // If external window, must ask its dimensions instead of trying to set them
            SDL_GetWindowSize(window_, &newWidth, &newHeight);
            newFullscreen = false;
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

        impl_->depthStencilView_ = nullptr;
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

    void Graphics::SetTexture(unsigned index, Texture* texture)
    {
        if (index >= MAX_TEXTURE_UNITS)
            return;

        // Check if texture is currently bound as a rendertarget. In that case, use its backup texture, or blank if not defined
        if (texture)
        {
            if (renderTargets_[0] && renderTargets_[0]->GetParentTexture() == texture)
                texture = texture->GetBackupTexture();
            else
            {
                // Resolve multisampled texture now as necessary
                if (texture->GetMultiSample() > 1 && texture->GetAutoResolve() && texture->IsResolveDirty())
                {
                    if (texture->GetType() == Texture2D::GetTypeStatic())
                        ResolveToTexture(static_cast<Texture2D*>(texture));
                    if (texture->GetType() == TextureCube::GetTypeStatic())
                        ResolveToTexture(static_cast<TextureCube*>(texture));
                }
            }

            if (texture && texture->GetLevelsDirty())
                texture->RegenerateLevels();
        }

        if (texture && texture->GetParametersDirty())
        {
            texture->UpdateParameters();
            textures_[index] = nullptr; // Force reassign
        }

        if (texture != textures_[index])
        {
            if (impl_->firstDirtyTexture_ == M_MAX_UNSIGNED)
                impl_->firstDirtyTexture_ = impl_->lastDirtyTexture_ = index;
            else
            {
                if (index < impl_->firstDirtyTexture_)
                    impl_->firstDirtyTexture_ = index;
                if (index > impl_->lastDirtyTexture_)
                    impl_->lastDirtyTexture_ = index;
            }

            textures_[index] = texture;
            impl_->shaderResourceViews_[index] = texture ? (ID3D11ShaderResourceView*)texture->GetShaderResourceView() : nullptr;
            impl_->samplers_[index] = texture ? (ID3D11SamplerState*)texture->GetSampler() : nullptr;
            impl_->texturesDirty_ = true;
        }
    }

    void Graphics::ResetRenderTargets()
    {
        for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
            SetRenderTarget(i, (RenderSurface*)nullptr);
        SetDepthStencil((RenderSurface*)nullptr);
        SetViewport(IntRect(0, 0, width_, height_));
    }

    void Graphics::ResetRenderTarget(unsigned index)
    {
        SetRenderTarget(index, (RenderSurface*)nullptr);
    }

    void Graphics::ResetDepthStencil()
    {
        SetDepthStencil((RenderSurface*)nullptr);
    }

    void Graphics::SetRenderTarget(unsigned index, RenderSurface* renderTarget)
    {
        if (index >= MAX_RENDERTARGETS)
            return;

        if (renderTarget != renderTargets_[index])
        {
            renderTargets_[index] = renderTarget;
            impl_->renderTargetsDirty_ = true;

            // If the rendertarget is also bound as a texture, replace with backup texture or null
            if (renderTarget)
            {
                Texture* parentTexture = renderTarget->GetParentTexture();

                for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
                {
                    if (textures_[i] == parentTexture)
                        SetTexture(i, textures_[i]->GetBackupTexture());
                }

                // If multisampled, mark the texture & surface needing resolve
                if (parentTexture->GetMultiSample() > 1 && parentTexture->GetAutoResolve())
                {
                    parentTexture->SetResolveDirty(true);
                    renderTarget->SetResolveDirty(true);
                }

                // If mipmapped, mark the levels needing regeneration
                if (parentTexture->GetLevels() > 1)
                    parentTexture->SetLevelsDirty();
            }
        }
    }

    void Graphics::SetDepthStencil(RenderSurface* depthStencil)
    {
        if (depthStencil != depthStencil_)
        {
            depthStencil_ = depthStencil;
            impl_->renderTargetsDirty_ = true;
        }
    }

    void Graphics::SetDepthStencil(Texture2D* texture)
    {
        RenderSurface* depthStencil = nullptr;
        if (texture)
            depthStencil = texture->GetRenderSurface();

        SetDepthStencil(depthStencil);
        // Constant depth bias depends on the bitdepth
        impl_->rasterizerStateDirty_ = true;
    }

    void Graphics::SetViewport(const IntRect& rect)
    {
        IntVector2 size = GetRenderTargetDimensions();

        IntRect rectCopy = rect;

        if (rectCopy.right_ <= rectCopy.left_)
            rectCopy.right_ = rectCopy.left_ + 1;
        if (rectCopy.bottom_ <= rectCopy.top_)
            rectCopy.bottom_ = rectCopy.top_ + 1;
        rectCopy.left_ = Clamp(rectCopy.left_, 0, size.x_);
        rectCopy.top_ = Clamp(rectCopy.top_, 0, size.y_);
        rectCopy.right_ = Clamp(rectCopy.right_, 0, size.x_);
        rectCopy.bottom_ = Clamp(rectCopy.bottom_, 0, size.y_);

        static D3D11_VIEWPORT d3dViewport;
        d3dViewport.TopLeftX = (float)rectCopy.left_;
        d3dViewport.TopLeftY = (float)rectCopy.top_;
        d3dViewport.Width = (float)(rectCopy.right_ - rectCopy.left_);
        d3dViewport.Height = (float)(rectCopy.bottom_ - rectCopy.top_);
        d3dViewport.MinDepth = 0.0f;
        d3dViewport.MaxDepth = 1.0f;

        impl_->deviceContext_->RSSetViewports(1, &d3dViewport);

        viewport_ = rectCopy;

        // Disable scissor test, needs to be re-enabled by the user
        SetScissorTest(false);
    }

    void Graphics::SetScissorTest(bool enable, const Rect& rect, bool borderInclusive)
    {
        // During some light rendering loops, a full rect is toggled on/off repeatedly.
        // Disable scissor in that case to reduce state changes
        if (rect.min_.x_ <= 0.0f && rect.min_.y_ <= 0.0f && rect.max_.x_ >= 1.0f && rect.max_.y_ >= 1.0f)
            enable = false;

        if (enable) {
            IntVector2 rtSize(GetRenderTargetDimensions());
            IntVector2 viewSize(viewport_.Size());
            IntVector2 viewPos(viewport_.left_, viewport_.top_);
            IntRect intRect;
            int expand = borderInclusive ? 1 : 0;

            intRect.left_ = Clamp((int) ((rect.min_.x_ + 1.0f) * 0.5f * viewSize.x_) + viewPos.x_, 0, rtSize.x_ - 1);
            intRect.top_ = Clamp((int) ((-rect.max_.y_ + 1.0f) * 0.5f * viewSize.y_) + viewPos.y_, 0, rtSize.y_ - 1);
            intRect.right_ = Clamp((int) ((rect.max_.x_ + 1.0f) * 0.5f * viewSize.x_) + viewPos.x_ + expand, 0,
                                   rtSize.x_);
            intRect.bottom_ = Clamp((int) ((-rect.min_.y_ + 1.0f) * 0.5f * viewSize.y_) + viewPos.y_ + expand, 0,
                                    rtSize.y_);

            if (intRect.right_ == intRect.left_)
                intRect.right_++;
            if (intRect.bottom_ == intRect.top_)
                intRect.bottom_++;

            if (intRect.right_ < intRect.left_ || intRect.bottom_ < intRect.top_)
                enable = false;

            if (enable && intRect != scissorRect_) {
                scissorRect_ = intRect;
                impl_->scissorRectDirty_ = true;
            }
        }

        if (enable != scissorTest_) {
            scissorTest_ = enable;
            impl_->rasterizerStateDirty_ = true;
        }
    }

    void Graphics::SetScissorTest(bool enable, const IntRect& rect)
    {
        IntVector2 rtSize(GetRenderTargetDimensions());
        IntVector2 viewPos(viewport_.left_, viewport_.top_);

        if (enable)
        {
            IntRect intRect;
            intRect.left_ = Clamp(rect.left_ + viewPos.x_, 0, rtSize.x_ - 1);
            intRect.top_ = Clamp(rect.top_ + viewPos.y_, 0, rtSize.y_ - 1);
            intRect.right_ = Clamp(rect.right_ + viewPos.x_, 0, rtSize.x_);
            intRect.bottom_ = Clamp(rect.bottom_ + viewPos.y_, 0, rtSize.y_);

            if (intRect.right_ == intRect.left_)
                intRect.right_++;
            if (intRect.bottom_ == intRect.top_)
                intRect.bottom_++;

            if (intRect.right_ < intRect.left_ || intRect.bottom_ < intRect.top_)
                enable = false;

            if (enable && intRect != scissorRect_)
            {
                scissorRect_ = intRect;
                impl_->scissorRectDirty_ = true;
            }
        }

        if (enable != scissorTest_)
        {
            scissorTest_ = enable;
            impl_->rasterizerStateDirty_ = true;
        }
    }

    void Graphics::SetBlendMode(BlendMode mode, bool alphaToCoverage)
    {
        if (mode != blendMode_ || alphaToCoverage != alphaToCoverage_)
        {
            blendMode_ = mode;
            alphaToCoverage_ = alphaToCoverage;
            impl_->blendStateDirty_ = true;
        }
    }

    void Graphics::SetColorWrite(bool enable)
    {
        if (enable != colorWrite_)
        {
            colorWrite_ = enable;
            impl_->blendStateDirty_ = true;
        }
    }

    void Graphics::SetCullMode(CullMode mode)
    {
        if (mode != cullMode_)
        {
            cullMode_ = mode;
            impl_->rasterizerStateDirty_ = true;
        }
    }

    void Graphics::SetDepthBias(float constantBias, float slopeScaledBias)
    {
        if (constantBias != constantDepthBias_ || slopeScaledBias != slopeScaledDepthBias_)
        {
            constantDepthBias_ = constantBias;
            slopeScaledDepthBias_ = slopeScaledBias;
            impl_->rasterizerStateDirty_ = true;
        }
    }

    void Graphics::SetDepthTest(CompareMode mode)
    {
        if (mode != depthTestMode_)
        {
            depthTestMode_ = mode;
            impl_->depthStateDirty_ = true;
        }
    }

    void Graphics::SetDepthWrite(bool enable)
    {
        if (enable != depthWrite_)
        {
            depthWrite_ = enable;
            impl_->depthStateDirty_ = true;
            // Also affects whether a read-only version of depth-stencil should be bound, to allow sampling
            impl_->renderTargetsDirty_ = true;
        }
    }

    void Graphics::SetFillMode(FillMode mode)
    {
        if (mode != fillMode_)
        {
            fillMode_ = mode;
            impl_->rasterizerStateDirty_ = true;
        }
    }

    void Graphics::SetLineAntiAlias(bool enable)
    {
        if (enable != lineAntiAlias_)
        {
            lineAntiAlias_ = enable;
            impl_->rasterizerStateDirty_ = true;
        }
    }

    bool Graphics::IsInitialized() const
    {
        return window_ != nullptr && impl_->GetDevice() != nullptr;
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

    unsigned Graphics::GetFormat(CompressedFormat format) const
    {
        switch (format)
        {
            case CF_RGBA:
                return DXGI_FORMAT_R8G8B8A8_UNORM;

            case CF_DXT1:
                return DXGI_FORMAT_BC1_UNORM;

            case CF_DXT3:
                return DXGI_FORMAT_BC2_UNORM;

            case CF_DXT5:
                return DXGI_FORMAT_BC3_UNORM;

            default:
                return 0;
        }
    }

    VertexBuffer* Graphics::GetVertexBuffer(unsigned index) const
    {
        return index < MAX_VERTEX_STREAMS ? vertexBuffers_[index] : nullptr;
    }

    TextureUnit Graphics::GetTextureUnit(const String& name)
    {
        HashMap<String, TextureUnit>::Iterator i = textureUnits_.Find(name);
        if (i != textureUnits_.End())
            return i->second_;
        else
            return MAX_TEXTURE_UNITS;
    }

    const String& Graphics::GetTextureUnitName(TextureUnit unit)
    {
        for (HashMap<String, TextureUnit>::Iterator i = textureUnits_.Begin(); i != textureUnits_.End(); ++i)
        {
            if (i->second_ == unit)
                return i->first_;
        }
        return String::EMPTY;
    }

    Texture* Graphics::GetTexture(unsigned index) const
    {
        return index < MAX_TEXTURE_UNITS ? textures_[index] : nullptr;
    }

    RenderSurface* Graphics::GetRenderTarget(unsigned index) const
    {
        return index < MAX_RENDERTARGETS ? renderTargets_[index] : nullptr;
    }

    bool Graphics::GetDither() const
    {
        return false;
    }

    bool Graphics::IsDeviceLost() const
    {
        // Direct3D11 graphics context is never considered lost
        return false;
    }

    void Graphics::OnWindowResized()
    {
        if (!impl_->device_ || !window_)
            return;

        int newWidth, newHeight;

        SDL_GetWindowSize(window_, &newWidth, &newHeight);
        if (newWidth == width_ && newHeight == height_)
            return;

        UpdateSwapChain(newWidth, newHeight);

        // Reset rendertargets and viewport for the new screen size
        ResetRenderTargets();

        MY3D_LOGDEBUGF("Window was resized to %dx%d", width_, height_);

        using namespace ScreenMode;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_WIDTH] = width_;
        eventData[P_HEIGHT] = height_;
        eventData[P_FULLSCREEN] = screenParams_.fullscreen_;
        eventData[P_RESIZABLE] = screenParams_.resizable_;
        eventData[P_BORDERLESS] = screenParams_.borderless_;
        eventData[P_HIGHDPI] = screenParams_.highDPI_;
        SendEvent(E_SCREENMODE, eventData);
    }

    void Graphics::OnWindowMoved()
    {
        if (!impl_->device_ || !window_ || screenParams_.fullscreen_)
            return;

        int newX, newY;

        SDL_GetWindowPosition(window_, &newX, &newY);
        if (newX == position_.x_ && newY == position_.y_)
            return;

        position_.x_ = newX;
        position_.y_ = newY;

        MY3D_LOGTRACEF("Window was moved to %d,%d", position_.x_, position_.y_);

        using namespace WindowPos;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_X] = position_.x_;
        eventData[P_Y] = position_.y_;
        SendEvent(E_WINDOWPOS, eventData);
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

    void Graphics::CheckFeatureSupport()
    {
        anisotropySupport_ = true;
        dxtTextureSupport_ = true;
        lightPrepassSupport_ = true;
        deferredSupport_ = true;
        hardwareShadowSupport_ = true;
        instancingSupport_ = true;
        shadowMapFormat_ = DXGI_FORMAT_R16_TYPELESS;
        hiresShadowMapFormat_ = DXGI_FORMAT_R32_TYPELESS;
        dummyColorFormat_ = DXGI_FORMAT_UNKNOWN;
        sRGBSupport_ = true;
        sRGBWriteSupport_ = true;
    }

    void Graphics::ResetCachedState()
    {
        for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
        {
            vertexBuffers_[i] = nullptr;
            impl_->vertexBuffers_[i] = nullptr;
            impl_->vertexSizes_[i] = 0;
            impl_->vertexOffsets_[i] = 0;
        }

        for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
        {
            textures_[i] = nullptr;
            impl_->shaderResourceViews_[i] = nullptr;
            impl_->samplers_[i] = nullptr;
        }

        for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
        {
            renderTargets_[i] = nullptr;
            impl_->renderTargetViews_[i] = nullptr;
        }

        depthStencil_ = nullptr;
        impl_->depthStencilView_ = nullptr;
        viewport_ = IntRect(0, 0, width_, height_);

    }

    void Graphics::SetTextureUnitMappings()
    {
        textureUnits_["DiffMap"] = TU_DIFFUSE;
        textureUnits_["DiffCubeMap"] = TU_DIFFUSE;
        textureUnits_["NormalMap"] = TU_NORMAL;
        textureUnits_["SpecMap"] = TU_SPECULAR;
        textureUnits_["EmissiveMap"] = TU_EMISSIVE;
        textureUnits_["EnvMap"] = TU_ENVIRONMENT;
        textureUnits_["EnvCubeMap"] = TU_ENVIRONMENT;
        textureUnits_["LightRampMap"] = TU_LIGHTRAMP;
        textureUnits_["LightSpotMap"] = TU_LIGHTSHAPE;
        textureUnits_["LightCubeMap"] = TU_LIGHTSHAPE;
        textureUnits_["ShadowMap"] = TU_SHADOWMAP;
        textureUnits_["FaceSelectCubeMap"] = TU_FACESELECT;
        textureUnits_["IndirectionCubeMap"] = TU_INDIRECTION;
        textureUnits_["VolumeMap"] = TU_VOLUMEMAP;
        textureUnits_["ZoneCubeMap"] = TU_ZONE;
        textureUnits_["ZoneVolumeMap"] = TU_ZONE;
    }

    void Graphics::CreateResolveTexture()
    {
        if (impl_->resolveTexture_)
            return;

        D3D11_TEXTURE2D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof textureDesc);
        textureDesc.Width = (UINT)width_;
        textureDesc.Height = (UINT)height_;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.CPUAccessFlags = 0;

        HRESULT hr = impl_->device_->CreateTexture2D(&textureDesc, nullptr, &impl_->resolveTexture_);
        if (FAILED(hr))
        {
            MY3D_SAFE_RELEASE(impl_->resolveTexture_);
            MY3D_LOGD3DERROR("Could not create resolve texture", hr);
        }
    }
}
