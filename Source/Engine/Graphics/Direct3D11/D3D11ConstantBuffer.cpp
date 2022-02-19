//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "Graphics/ConstantBuffer.h"
#include "IO/Log.h"


namespace My3D
{
    void ConstantBuffer::OnDeviceReset()
    {
        // No-op on Direct3D11
    }

    void ConstantBuffer::Release()
    {
        MY3D_SAFE_RELEASE(object_.ptr_);

        shadowData_.Reset();
        size_ = 0;
    }

    bool ConstantBuffer::SetSize(unsigned int size)
    {
        Release();

        if (!size)
        {
            MY3D_LOGERROR("Can not create zero-sized constant buffer");
            return false;
        }

        // Round up to next 16 bytes
        size += 15;
        size &= 0xfffffff0;

        size_ = size;
        dirty_ = false;
        shadowData_ = new unsigned char[size_];
        memset(shadowData_.Get(), 0, size_);

        if (graphics_)
        {
            D3D11_BUFFER_DESC bufferDesc;
            memset(&bufferDesc, 0, sizeof bufferDesc);

            bufferDesc.ByteWidth = size_;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = 0;
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;

            HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateBuffer(&bufferDesc, 0, (ID3D11Buffer**)&object_.ptr_);
            if (FAILED(hr))
            {
                MY3D_SAFE_RELEASE(object_.ptr_);
                MY3D_LOGD3DERROR("Failed to create constant buffer", hr);
                return false;
            }
        }

        return true;
    }

    void ConstantBuffer::Apply()
    {
        if (dirty_ && object_.ptr_)
        {
            graphics_->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*)object_.ptr_, 0, 0, shadowData_.Get(), 0, 0);
            dirty_ = false;
        }
    }
}