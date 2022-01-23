//
// Created by luchu on 2022/1/23.
//

#include "MemoryBuffer.h"

namespace My3D
{
    MemoryBuffer::MemoryBuffer(void *data, unsigned int size)
        : AbstractFile(size)
        , buffer_((unsigned char*) data)
        , readOnly_(false)
    {
        if (!buffer_)
            size_ = 0;
    }

    MemoryBuffer::MemoryBuffer(const void *data, unsigned int size)
        : AbstractFile(size)
        , buffer_((unsigned char*) data)
        , readOnly_(false)
    {
        if (!buffer_)
            size_ = 0;
    }

    MemoryBuffer::MemoryBuffer(PODVector<unsigned char> &data)
        : AbstractFile(data.Size())
        , buffer_(data.Begin().ptr_)
        , readOnly_(false)
    {
    }

    MemoryBuffer::MemoryBuffer(const PODVector<unsigned char> &data)
        : AbstractFile(data.Size())
        , buffer_(const_cast<unsigned char*>(data.Begin().ptr_))
        , readOnly_(true)
    {
    }

    unsigned MemoryBuffer::Read(void *dest, unsigned int size)
    {
        if (size + position_ > size_)
            size = size_ - position_;
        if (!size)
            return 0;

        unsigned char* srcPtr = &buffer_[position_];
        auto* destPtr = (unsigned char*) dest;
        position_ += size;

        memcpy(destPtr, srcPtr, size);

        return size;
    }

    unsigned MemoryBuffer::Seek(unsigned int position)
    {
        if (position > size_)
            position = size_;

        position_ = position;
        return position_;
    }

    unsigned MemoryBuffer::Write(const void *data, unsigned int size)
    {
        if (size + position_ > size_)
            size = size_ - position_;
        if (!size)
            return 0;

        auto* srcPtr = (unsigned char*)data;
        unsigned char* destPtr = &buffer_[position_];
        position_ += size;

        memcpy(destPtr, srcPtr, size);

        return size;
    }
}