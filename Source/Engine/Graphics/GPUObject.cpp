//
// Created by luchu on 2022/1/21.
//

#include "Graphics/Graphics.h"
#include "Graphics/GPUObject.h"

namespace My3D
{
    GPUObject::GPUObject(Graphics *graphics)
        : graphics_(graphics)
    {
        object_.ptr_ = nullptr;

        if (graphics_)
            graphics_->AddGPUObject(this);
    }

    GPUObject::~GPUObject()
    {
        if (graphics_)
            graphics_->RemoveGPUObject(this);
    }

    void GPUObject::OnDeviceLost()
    {
    }

    void GPUObject::OnDeviceReset()
    {
    }

    void GPUObject::Release()
    {
    }

    void GPUObject::ClearDataLost()
    {
        dataLost_ = false;
    }

    Graphics* GPUObject::GetGraphics() const
    {
        return graphics_;
    }
}

