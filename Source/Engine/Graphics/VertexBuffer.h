//
// Created by luchu on 2022/1/21.
//

#pragma once

#include "Core/Object.h"
#include "Graphics/GPUObject.h"
#include "Graphics/GraphicsDefs.h"
#include "Container/ArrayPtr.h"


namespace My3D
{
    /// Hardware vertex buffer
    class MY3D_API VertexBuffer : public Object, public GPUObject
    {
        MY3D_OBJECT(VertexBuffer, Object)

    public:
        /// Construct
        explicit VertexBuffer(Context* context, bool forceHeadless = true);
        /// Destruct
        ~VertexBuffer() override;
        /// Mark the buffer destroyed on graphics context destruction. May be a no-op depending on the API.
        void OnDeviceLost() override;
        /// Recreate the buffer and restore data if applicable. May be a no-op depending on the API.
        void OnDeviceReset() override;
        /// Release buffer.
        void Release() override;

        /// Set all data in the buffer
        bool SetData(const void* data);

    private:
        /// Update offsets of vertex elements
        void UpdateOffsets();
        /// Create buffer
        bool Create();
        /// Update the shadow data to the GPU buffer
        bool UpdateToGPU();
        /// Map the GPU buffer into CPU memory. Not used on OpenGL
        void* MapBuffer(unsigned start, unsigned count, bool discard);
        /// Unmap the GPU buffer. Not used on OpenGL
        void UnmapBuffer();

        /// Shadow data
        SharedArrayPtr<unsigned char> shadowData_;
        /// Number of vertices
        unsigned vertexCount_;
        /// Vertex size
        unsigned vertexSize_;
        /// Vertex elements
        PODVector<VertexElement> elements_;
        /// Vertex element hash.
        unsigned long long elementHash_{};
        /// Dynamic flag
        bool dynamic_{};
        /// Shadowed flag
        bool shadowed_{};
        /// Discard lock flag (OpenGL only).
        bool discardLock_{};
    };
}
