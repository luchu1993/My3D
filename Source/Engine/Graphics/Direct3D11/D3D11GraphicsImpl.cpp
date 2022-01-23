//
// Created by luchu on 2022/1/18.
//

#include "Graphics/Graphics.h"
#include "Graphics/Direct3D11/D3D11GraphicsImpl.h"

namespace My3D
{
    GraphicsImpl::GraphicsImpl()
        : device_(nullptr)
        , deviceContext_(nullptr)
        , swapChain_(nullptr)
        , defaultRenderTargetView_(nullptr)
        , defaultDepthTexture_(nullptr)
        , defaultDepthStencilView_(nullptr)
        , depthStencilView_(nullptr)
        , resolveTexture_(nullptr)
    {
        for (unsigned i = 0; i < MAX_RENDERTARGETS; ++i)
            renderTargetViews_[i] = nullptr;

        for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
        {
            shaderResourceViews_[i] = nullptr;
            samplers_[i] = nullptr;
        }

        for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
        {
            vertexBuffers_[i] = nullptr;
            vertexSizes_[i] = 0;
            vertexOffsets_[i] = 0;
        }
    }

    bool GraphicsImpl::CheckMultiSampleSupport(DXGI_FORMAT format, unsigned int sampleCount) const
    {
        if (sampleCount < 2)
            return true;

        UINT numLevels = 0;
        if (FAILED(device_->CheckMultisampleQualityLevels(format, sampleCount, &numLevels)))
            return false;
        else
            return numLevels > 0;
    }

    unsigned GraphicsImpl::GetMultiSampleQuality(DXGI_FORMAT format, unsigned int sampleCount) const
    {
        if (sampleCount < 2)
            return 0; // Not multisampled, should use quality 0

        if (device_->GetFeatureLevel() >= D3D_FEATURE_LEVEL_10_1)
            return 0xffffffff; // D3D10.1+ standard level

        UINT numLevels = 0;
        if (FAILED(device_->CheckMultisampleQualityLevels(format, sampleCount, &numLevels)) || !numLevels)
            return 0; // Errored or sample count not supported
        else
            return numLevels - 1; // D3D10.0 and below: use the best quality
    }
}