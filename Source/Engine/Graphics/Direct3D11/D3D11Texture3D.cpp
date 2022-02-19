//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture3D.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"


namespace My3D
{
    void Texture3D::OnDeviceLost()
    {
        // No-op on Direct3D11
    }

    void Texture3D::OnDeviceReset()
    {
        // No-op on Direct3D11
    }

    void Texture3D::Release()
    {
        if (graphics_ && object_.ptr_)
        {
            for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
            {
                if (graphics_->GetTexture(i) == this)
                    graphics_->SetTexture(i, nullptr);
            }
        }

        MY3D_SAFE_RELEASE(object_.ptr_);
        MY3D_SAFE_RELEASE(shaderResourceView_);
        MY3D_SAFE_RELEASE(sampler_);
    }

    bool Texture3D::Create()
    {
        Release();

        if (!graphics_ || !width_ || !height_ || !depth_)
            return false;

        levels_ = CheckMaxLevels(width_, height_, depth_, requestedLevels_);

        D3D11_TEXTURE3D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof textureDesc);
        textureDesc.Width = (UINT)width_;
        textureDesc.Height = (UINT)height_;
        textureDesc.Depth = (UINT)depth_;
        textureDesc.MipLevels = usage_ != TEXTURE_DYNAMIC ? levels_ : 1;
        textureDesc.Format = (DXGI_FORMAT)(sRGB_ ? GetSRGBFormat(format_) : format_);
        textureDesc.Usage = usage_ == TEXTURE_DYNAMIC ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = usage_ == TEXTURE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;

        HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateTexture3D(&textureDesc, nullptr, (ID3D11Texture3D**)&object_.ptr_);
        if (FAILED(hr))
        {
            MY3D_LOGD3DERROR("Failed to create texture", hr);
            MY3D_SAFE_RELEASE(object_.ptr_);
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
        memset(&resourceViewDesc, 0, sizeof resourceViewDesc);
        resourceViewDesc.Format = (DXGI_FORMAT)GetSRVFormat(textureDesc.Format);
        resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        resourceViewDesc.Texture3D.MipLevels = usage_ != TEXTURE_DYNAMIC ? (UINT)levels_ : 1;

        hr = graphics_->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Resource*)object_.ptr_, &resourceViewDesc,
                                                                         (ID3D11ShaderResourceView**)&shaderResourceView_);
        if (FAILED(hr))
        {
            MY3D_LOGD3DERROR("Failed to create shader resource view for texture", hr);
            MY3D_SAFE_RELEASE(shaderResourceView_);
            return false;
        }

        return true;
    }
}

