//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/ConstantBuffer.h"


namespace My3D
{
    ConstantBuffer::ConstantBuffer(Context* context)
        : Object(context)
        , GPUObject(GetSubsystem<Graphics>())
    {
    }

    ConstantBuffer::~ConstantBuffer()
    {
        Release();
    }

    void ConstantBuffer::SetParameter(unsigned offset, unsigned size, const void* data)
    {
        if (offset + size > size_)
            return; // Would overflow the buffer

        memcpy(&shadowData_[offset], data, size);
        dirty_ = true;
    }

    void ConstantBuffer::SetVector3ArrayParameter(unsigned offset, unsigned rows, const void* data)
    {
        if (offset + rows * 4 * sizeof(float) > size_)
            return; // Would overflow the buffer

        auto* dest = (float*)&shadowData_[offset];
        const auto* src = (const float*)data;

        while (rows--)
        {
            *dest++ = *src++;
            *dest++ = *src++;
            *dest++ = *src++;
            ++dest; // Skip over the w coordinate
        }

        dirty_ = true;
    }
}