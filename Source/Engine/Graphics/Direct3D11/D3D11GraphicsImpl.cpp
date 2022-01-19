//
// Created by luchu on 2022/1/18.
//

#include "Graphics/Graphics.h"
#include "Graphics/Direct3D11/D3D11GraphicsImpl.h"

namespace My3D
{
    GraphicsImpl::GraphicsImpl()
    {

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