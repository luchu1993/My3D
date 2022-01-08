//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Container/Swap.h"


namespace My3D
{

class MY3D_API VectorBase
{
public:
    VectorBase() noexcept : size_(0), capacity_(0), buffer_(nullptr) { }

    void Swap(VectorBase& rhs)
    {
        My3D::Swap(size_, rhs.size_);
        My3D::Swap(capacity_, rhs.capacity_);
        My3D::Swap(buffer_, rhs.buffer_);
    }

protected:
    static unsigned char* AllocateBuffer(unsigned size);

    unsigned size_;
    unsigned capacity_;
    unsigned char* buffer_;
};

}
