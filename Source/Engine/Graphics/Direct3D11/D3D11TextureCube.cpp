//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/Renderer.h"
#include "Graphics/TextureCube.h"
#include "IO/FileSystem.h"
#include "IO/Log.h"
#include "Resource/ResourceCache.h"


namespace My3D
{
    void TextureCube::OnDeviceLost()
    {
        // No-op on Direct3D11
    }

    void TextureCube::OnDeviceReset()
    {
        // No-op on Direct3D11
    }

    void TextureCube::Release()
    {
        if (graphics_)
        {
            for (unsigned i = 0; i < MAX_TEXTURE_UNITS; ++i)
            {
                if (graphics_->GetTexture(i) == this)
                    graphics_->SetTexture(i, nullptr);
            }
        }

        for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
        {
            if (renderSurfaces_[i])
                renderSurfaces_[i]->Release();
        }

        MY3D_SAFE_RELEASE(object_.ptr_);
        MY3D_SAFE_RELEASE(resolveTexture_);
        MY3D_SAFE_RELEASE(shaderResourceView_);
        MY3D_SAFE_RELEASE(sampler_);
    }

    bool TextureCube::Create()
    {
        Release();

        if (!graphics_ || !width_ || !height_)
            return false;

        levels_ = CheckMaxLevels(width_, height_, requestedLevels_);

        D3D11_TEXTURE2D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof textureDesc);
        textureDesc.Format = (DXGI_FORMAT)(sRGB_ ? GetSRGBFormat(format_) : format_);

        // Disable multisampling if not supported
        if (multiSample_ > 1 && !graphics_->GetImpl()->CheckMultiSampleSupport(textureDesc.Format, multiSample_))
        {
            multiSample_ = 1;
            autoResolve_ = false;
        }

        // Set mipmapping
        if (usage_ == TEXTURE_RENDERTARGET && levels_ != 1 && multiSample_ == 1)
            textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

        textureDesc.Width = (UINT)width_;
        textureDesc.Height = (UINT)height_;
        // Disable mip levels from the multisample texture. Rather create them to the resolve texture
        textureDesc.MipLevels = (multiSample_ == 1 && usage_ != TEXTURE_DYNAMIC) ? levels_ : 1;
        textureDesc.ArraySize = MAX_CUBEMAP_FACES;
        textureDesc.SampleDesc.Count = (UINT)multiSample_;
        textureDesc.SampleDesc.Quality = graphics_->GetImpl()->GetMultiSampleQuality(textureDesc.Format, multiSample_);
        textureDesc.Usage = usage_ == TEXTURE_DYNAMIC ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        if (usage_ == TEXTURE_RENDERTARGET)
            textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
        else if (usage_ == TEXTURE_DEPTHSTENCIL)
            textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
        textureDesc.CPUAccessFlags = usage_ == TEXTURE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;
        // When multisample is specified, creating an actual cube texture will fail. Rather create as a 2D texture array
        // whose faces will be rendered to; only the resolve texture will be an actual cube texture
        if (multiSample_ < 2)
            textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

        HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateTexture2D(&textureDesc, nullptr, (ID3D11Texture2D**)&object_.ptr_);
        if (FAILED(hr))
        {
            MY3D_LOGD3DERROR("Failed to create texture", hr);
            MY3D_SAFE_RELEASE(object_.ptr_);
            return false;
        }

        // Create resolve texture for multisampling
        if (multiSample_ > 1)
        {
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
            if (levels_ != 1)
                textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

            HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateTexture2D(&textureDesc, nullptr, (ID3D11Texture2D**)&resolveTexture_);
            if (FAILED(hr))
            {
                MY3D_LOGD3DERROR("Failed to create resolve texture", hr);
                MY3D_SAFE_RELEASE(resolveTexture_);
                return false;
            }
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
        memset(&resourceViewDesc, 0, sizeof resourceViewDesc);
        resourceViewDesc.Format = (DXGI_FORMAT)GetSRVFormat(textureDesc.Format);
        resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        resourceViewDesc.Texture2D.MipLevels = usage_ != TEXTURE_DYNAMIC ? (UINT)levels_ : 1;

        // Sample the resolve texture if created, otherwise the original
        ID3D11Resource* viewObject = resolveTexture_ ? (ID3D11Resource*)resolveTexture_ : (ID3D11Resource*)object_.ptr_;
        hr = graphics_->GetImpl()->GetDevice()->CreateShaderResourceView(viewObject, &resourceViewDesc,
                                                                         (ID3D11ShaderResourceView**)&shaderResourceView_);
        if (FAILED(hr))
        {
            MY3D_LOGD3DERROR("Failed to create shader resource view for texture", hr);
            MY3D_SAFE_RELEASE(shaderResourceView_);
            return false;
        }

        if (usage_ == TEXTURE_RENDERTARGET)
        {
            for (unsigned i = 0; i < MAX_CUBEMAP_FACES; ++i)
            {
                D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
                memset(&renderTargetViewDesc, 0, sizeof renderTargetViewDesc);
                renderTargetViewDesc.Format = textureDesc.Format;
                if (multiSample_ > 1)
                {
                    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    renderTargetViewDesc.Texture2DMSArray.ArraySize = 1;
                    renderTargetViewDesc.Texture2DMSArray.FirstArraySlice = i;
                }
                else
                {
                    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                    renderTargetViewDesc.Texture2DArray.ArraySize = 1;
                    renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
                    renderTargetViewDesc.Texture2DArray.MipSlice = 0;
                }

                hr = graphics_->GetImpl()->GetDevice()->CreateRenderTargetView((ID3D11Resource*)object_.ptr_, &renderTargetViewDesc,
                                                                               (ID3D11RenderTargetView**)&renderSurfaces_[i]->renderTargetView_);

                if (FAILED(hr))
                {
                    MY3D_LOGD3DERROR("Failed to create rendertarget view for texture", hr);
                    MY3D_SAFE_RELEASE(renderSurfaces_[i]->renderTargetView_);
                    return false;
                }
            }
        }

        return true;
    }
}