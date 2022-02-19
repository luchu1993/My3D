//
// Created by luchu on 2022/2/19.
//

#pragma once

#include "Core/Object.h"
#include "Container/ArrayPtr.h"
#include "Graphics/GPUObject.h"
#include "Graphics/GraphicsDefs.h"


namespace My3D
{
    /// Hardware index buffer
    class MY3D_API IndexBuffer : public Object, public GPUObject
    {
        MY3D_OBJECT(IndexBuffer, Object)
    public:
        /// Construct. Optionally force headless (no GPU-side buffer) operation.
        explicit IndexBuffer(Context* context, bool forceHeadless = true);
        /// Destruct
        ~IndexBuffer() override;

        /// Mark the buffer destroyed on graphics context destruction. May be a no-op depending on the API.
        void OnDeviceLost() override;
        /// Recreate the buffer and restore data if applicable. May be a no-op depending on the API.
        void OnDeviceReset() override;
        /// Release buffer.
        void Release();

        /// Enable shadowing in CPU memory. Shadowing is forced on if the graphics subsystem does not exist.
        void SetShadowed(bool enable);
        /// Set size and vertex elements and dynamic mode. Previous data will be lost.
        bool SetSize(unsigned indexCount, bool largeIndices, bool dynamic = false);
        /// Set all data in the buffer.
        bool SetData(const void* data);
        /// Set a data range in the buffer. Optionally discard data outside the range.
        bool SetDataRange(const void* data, unsigned start, unsigned count, bool discard = false);
        /// Lock the buffer for write-only editing. Return data pointer if successful. Optionally discard data outside the range.
        void* Lock(unsigned start, unsigned count, bool discard = false);
        /// Unlock the buffer and apply changes to the GPU buffer.
        void Unlock();

        /// Return whether CPU memory shadowing is enabled.
        bool IsShadowed() const { return shadowed_; }
        /// Return whether is dynamic.
        bool IsDynamic() const { return dynamic_; }
        /// Return whether is currently locked.
        bool IsLocked() const { return lockState_ != LOCK_NONE; }
        /// Return number of indices.
        unsigned GetIndexCount() const { return indexCount_; }
        /// Return index size in bytes.
        unsigned GetIndexSize() const { return indexSize_; }
        /// Return used vertex range from index range.
        bool GetUsedVertexRange(unsigned start, unsigned count, unsigned& minVertex, unsigned& vertexCount);
        /// Return CPU memory shadow data.
        unsigned char* GetShadowData() const { return shadowData_.Get(); }
        /// Return shared array pointer to the CPU memory shadow data.
        SharedArrayPtr<unsigned char> GetShadowDataShared() const { return shadowData_; }

    private:
        /// Create buffer
        bool Create();
        /// Update shadow data to GPU buffer
        bool UpdateToGPU();
        /// Map GPU buffer into CPU memory. Not used in OpenGL
        void* MapBuffer(unsigned start, unsigned count, bool discard);
        /// Unmap GPU buffer. Not used on OpenGL
        void UnmapBuffer();

        /// Shadow data
        SharedArrayPtr<unsigned char> shadowData_;
        /// Number of indices
        unsigned indexCount_;
        /// Index size
        unsigned indexSize_;
        /// Buffer locking state.
        LockState lockState_;
        /// Lock start index.
        unsigned lockStart_;
        /// Lock number of indices.
        unsigned lockCount_;
        /// Scratch buffer for fallback locking.
        void* lockScratchData_;
        /// Dynamic flag.
        bool dynamic_;
        /// Shadowed flag.
        bool shadowed_;
        /// Discard lock flag. Used by OpenGL only.
        bool discardLock_;
    };
}