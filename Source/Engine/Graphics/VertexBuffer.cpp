//
// Created by luchu on 2022/1/21.
//

#include "Graphics/VertexBuffer.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsImpl.h"
#include "IO/Log.h"


namespace My3D
{
    void VertexBuffer::UpdateOffsets()
    {
        unsigned elementOffset = 0;
        elementHash_ = 0;

        for (auto& element : elements_)
        {
            element.offset_ = elementOffset;
            elementOffset += ELEMENT_TYPESIZES[element.type_];
            elementHash_ <<= 6;
            elementHash_ += (((int)element.type_ + 1) * ((int)element.semantic_ + 1) + element.index_);
        }

        vertexSize_ = elementOffset;
    }

    bool VertexBuffer::Create()
    {
        Release();

        if (!vertexCount_)
            return true;

        if (graphics_)
        {
            D3D11_BUFFER_DESC bufferDesc;
            memset(&bufferDesc, 0, sizeof bufferDesc);
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = dynamic_ ? D3D11_CPU_ACCESS_WRITE : 0;
            bufferDesc.Usage = dynamic_ ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = (UINT) (vertexCount_ * vertexSize_);

            HRESULT hr = graphics_->GetImpl()->GetDevice()->CreateBuffer(&bufferDesc, nullptr, (ID3D11Buffer**)&object_.ptr_);
            if (FAILED(hr))
            {
                MY3D_SAFE_RELEASE(object_.ptr_);
                MY3D_LOGD3DERROR("Failed to create vertex buffer", hr);
                return false;
            }
        }

        return true;
    }

    bool VertexBuffer::UpdateToGPU()
    {
        if (object_.ptr_ && shadowData_)
            return SetData(shadowData_.Get());
        else
            return false;
    }

    bool VertexBuffer::SetData(const void *data)
    {
        if (!data)
        {
            MY3D_LOGERROR("Null pointer for vertex buffer data");
            return false;
        }

        if (!vertexSize_)
        {
            MY3D_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
            return false;
        }

        if (shadowData_ && data != shadowData_.Get())
            memcpy(shadowData_.Get(), data, vertexCount_ * vertexSize_);

        if (object_.ptr_)
        {
            if (dynamic_)
            {
                void* hwData = MapBuffer(0, vertexCount_, true);
                if (hwData)
                {
                    memcpy(hwData, data, vertexCount_ * vertexSize_);
                    UnmapBuffer();
                }
                else
                    return false;
            }
            else
            {
                D3D11_BOX destBox;
                destBox.left = 0;
                destBox.right = vertexCount_ * vertexSize_;
                destBox.top = 0;
                destBox.bottom = 1;
                destBox.front = 0;
                destBox.back = 1;

                graphics_->GetImpl()->GetDeviceContext()->UpdateSubresource((ID3D11Buffer*) object_.ptr_, 0, &destBox, data, 0, 0);
            }
        }

        return true;
    }
}