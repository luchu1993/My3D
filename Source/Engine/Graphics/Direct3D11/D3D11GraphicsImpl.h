//
// Created by luchu on 2022/1/18.
//

#pragma once

#include "Math/Color.h"
#include <d3d11.h>
#include <dxgi.h>

namespace My3D
{
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
