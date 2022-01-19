//
// Created by luchu on 2022/1/18.
//

#pragma once

#include "Math/Color.h"
#include <d3d11.h>
#include <dxgi.h>

namespace My3D
{
#define MY3D_SAFE_RELEASE(p) if (p) { ((IUnknown*)p)->Release(); p = nullptr; }
#define MY3D_LOGD3DERROR(msg, hr) MY3D_LOGERRORF("%s (HRESULT %x)", msg, (unsigned) hr)

    class MY3D_API GraphicsImpl
    {
        friend class Graphics;

    public:
        /// Construct
        GraphicsImpl();
        /// Return Direct3D device
        ID3D11Device* GetDevice() { return device_; }
        /// Return Direct3D immdeiate device context
        ID3D11DeviceContext* GetDeviceContext() const { return deviceContext_; }
        /// Return swap chain
        IDXGISwapChain* GetSwapChain() const { return swapChain_; }
        /// Return whether multisampling is supported for a given texture format and sample count.
        bool CheckMultiSampleSupport(DXGI_FORMAT format, unsigned sampleCount) const;
        /// Return multisample quality level for a given texture format and sample count. The sample count must be supported. On D3D feature level 10.1+, uses the standard level. Below that uses the best quality.
        unsigned GetMultiSampleQuality(DXGI_FORMAT format, unsigned sampleCount) const;

    private:
        /// Graphics device
        ID3D11Device* device_;
        /// Immediate device context
        ID3D11DeviceContext* deviceContext_;
        /// Swap chain
        IDXGISwapChain* swapChain_;
        /// Default (backbuffer) rendertarget view.
        ID3D11RenderTargetView* defaultRenderTargetView_;
        /// Default depth-stencil texture
        ID3D11DepthStencilView* defaultDepthStencilView_;
    };
}
