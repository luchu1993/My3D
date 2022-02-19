//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture2DArray.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"


namespace My3D
{
    void Texture2DArray::OnDeviceLost()
    {
        // No-op on Direct3D11
    }

    void Texture2DArray::OnDeviceReset()
    {
        // No-op on Direct3D11
    }

    void Texture2DArray::Release()
    {
        if (graphics_)
        {
            for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
            {
                if (graphics_->GetTexture(i) == this)
                    graphics_->SetTexture(i, nullptr);
            }
        }

        if (renderSurface_)
            renderSurface_->Release();

        MY3D_SAFE_RELEASE(object_.ptr_);
        MY3D_SAFE_RELEASE(shaderResourceView_);
        MY3D_SAFE_RELEASE(sampler_);

        levelsDirty_ = false;
    }

    bool Texture2DArray::Create()
    {
        Release();

        if (!graphics_ || !width_ || !height_ || !layers_)
            return false;

        levels_ = CheckMaxLevels(width_, height_, requestedLevels_);

        D3D11_TEXTURE2D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof textureDesc);

        // Set mipmapping
        if (usage_ == TEXTURE_RENDERTARGET && levels_ != 1)
            textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

        textureDesc.Width = (UINT)width_;
        textureDesc.Height = (UINT)height_;
        textureDesc.MipLevels = usage_ != TEXTURE_DYNAMIC ? levels_ : 1;
        textureDesc.ArraySize = layers_;
        textureDesc.Format = (DXGI_FORMAT)(sRGB_ ? GetSRGBFormat(format_) : format_);
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = usage_ == TEXTURE_DYNAMIC ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (usage_ == TEXTURE_RENDERTARGET)
            textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        else if (usage_ == TEXTURE_DEPTHSTENCIL)
            textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = usage_ == TEXTURE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;

        HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateTexture2D(&textureDesc, nullptr, (ID3D11Texture2D**)&object_);
        if (FAILED(hr))
        {
            MY3D_LOGD3DERROR("Failed to create texture array", hr);
            MY3D_SAFE_RELEASE(object_.ptr_);
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        memset(&srvDesc, 0, sizeof srvDesc);
        srvDesc.Format = (DXGI_FORMAT)GetSRVFormat(textureDesc.Format);
        if (layers_ == 1)
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = usage_ != TEXTURE_DYNAMIC ? (UINT)levels_ : 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
        }
        else
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.MipLevels = usage_ != TEXTURE_DYNAMIC ? (UINT)levels_ : 1;
            srvDesc.Texture2DArray.ArraySize = layers_;
            srvDesc.Texture2DArray.FirstArraySlice = 0;
            srvDesc.Texture2DArray.MostDetailedMip = 0;
        }

        hr = graphics_->GetImpl()->GetDevice()->CreateShaderResourceView((ID3D11Resource*)object_.ptr_, &srvDesc,
                                                                         (ID3D11ShaderResourceView**)&shaderResourceView_);
        if (FAILED(hr))
        {
            MY3D_LOGD3DERROR("Failed to create shader resource view for texture array", hr);
            MY3D_SAFE_RELEASE(shaderResourceView_);
            return false;
        }

        if (usage_ == TEXTURE_RENDERTARGET)
        {
            D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
            memset(&renderTargetViewDesc, 0, sizeof renderTargetViewDesc);
            renderTargetViewDesc.Format = textureDesc.Format;
            if (layers_ == 1)
            {
                renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                renderTargetViewDesc.Texture2D.MipSlice = 0;
            }
            else
            {
                renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                renderTargetViewDesc.Texture2DArray.MipSlice = 0;
                renderTargetViewDesc.Texture2DArray.ArraySize = layers_;
                renderTargetViewDesc.Texture2DArray.FirstArraySlice = 0;
            }

            hr = graphics_->GetImpl()->GetDevice()->CreateRenderTargetView((ID3D11Resource*)object_.ptr_, &renderTargetViewDesc,
                                                                           (ID3D11RenderTargetView**)&renderSurface_->renderTargetView_);

            if (FAILED(hr))
            {
                MY3D_LOGD3DERROR("Failed to create rendertarget view for texture array", hr);
                MY3D_SAFE_RELEASE(renderSurface_->renderTargetView_);
                return false;
            }
        }

        return true;
    }

}