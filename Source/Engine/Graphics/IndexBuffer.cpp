//
// Created by luchu on 2022/2/19.
//

#include "Graphics/Graphics.h"
#include "Graphics/IndexBuffer.h"
#include "IO/Log.h"

namespace My3D
{
    IndexBuffer::IndexBuffer(Context *context, bool forceHeadless)
        : Object(context)
        , GPUObject(forceHeadless ? nullptr : GetSubsystem<Graphics>())
        , indexCount_(0)
        , indexSize_(0)
        , lockState_(LOCK_NONE)
        , lockStart_(0)
        , lockCount_(0)
        , lockScratchData_(nullptr)
        , shadowed_(false)
        , dynamic_(false)
        , discardLock_(false)
    {
        // Force shadowing mode if graphics subsystem does not exist
        if (!graphics_)
            shadowed_ = true;
    }

    IndexBuffer::~IndexBuffer()
    {
        Release();
    }

    void IndexBuffer::SetShadowed(bool enable)
    {
        // If no graphics subsystem, can not disable shadowing
        if (!graphics_)
            enable = true;

        if (enable != shadowed_)
        {
            if (enable && indexCount_ && indexSize_)
                shadowData_ = new unsigned char[indexCount_ * indexSize_];
            else
                shadowData_.Reset();

            shadowed_ = enable;
        }
    }

    bool IndexBuffer::SetSize(unsigned indexCount, bool largeIndices, bool dynamic)
    {
        Unlock();

        indexCount_ = indexCount;
        indexSize_ = (unsigned)(largeIndices ? sizeof(unsigned) : sizeof(unsigned short));
        dynamic_ = dynamic;

        if (shadowed_ && indexCount_ && indexSize_)
            shadowData_ = new unsigned char[indexCount_ * indexSize_];
        else
            shadowData_.Reset();

        return Create();
    }

    bool IndexBuffer::GetUsedVertexRange(unsigned start, unsigned count, unsigned& minVertex, unsigned& vertexCount)
    {
        if (!shadowData_)
        {
            MY3D_LOGERROR("Used vertex range can only be queried from an index buffer with shadow data");
            return false;
        }

        if (start + count > indexCount_)
        {
            MY3D_LOGERROR("Illegal index range for querying used vertices");
            return false;
        }

        minVertex = M_MAX_UNSIGNED;
        unsigned maxVertex = 0;

        if (indexSize_ == sizeof(unsigned))
        {
            unsigned* indices = ((unsigned*)shadowData_.Get()) + start;

            for (unsigned i = 0; i < count; ++i)
            {
                if (indices[i] < minVertex)
                    minVertex = indices[i];
                if (indices[i] > maxVertex)
                    maxVertex = indices[i];
            }
        }
        else
        {
            unsigned short* indices = ((unsigned short*)shadowData_.Get()) + start;

            for (unsigned i = 0; i < count; ++i)
            {
                if (indices[i] < minVertex)
                    minVertex = indices[i];
                if (indices[i] > maxVertex)
                    maxVertex = indices[i];
            }
        }

        vertexCount = maxVertex - minVertex + 1;
        return true;
    }
}
