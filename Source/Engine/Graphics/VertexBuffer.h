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

        /// Enable shadowing in CPU memory. Shadowing is forced on if the graphics subsystem does not exist
        void SetShadowed(bool enable);
        /// Set size, vertex elements and dynamic mode.
        bool SetSize(unsigned vertexCount, const PODVector<VertexElement>& elements, bool dynamic = false);
        /// Set all data in the buffer
        bool SetData(const void* data);
        /// Set a data range in teh buffer.
        bool SetDataRange(const void* data, unsigned start, unsigned count, bool discard = false);
        /// Lock the buffer for write-only editing,
        void* Lock(unsigned start, unsigned count, bool discard = false);
        /// Unlock the buffer and apply changes to the CPU buffer
        void Unlock();
        /// Return whether CPU memory shadowing is enabled.
        bool IsShadowed() const { return shadowed_; }
        /// Return whether is dynamic
        bool IsDynamic() const { return dynamic_; }
        /// Return whether is currently locked
        bool IsLocked() const { return lockState_ != LOCK_NONE; }
        /// Return number of vertices
        unsigned GetVertexCount() const { return vertexCount_; }
        /// Return vertex size in bytes
        unsigned GetVertexSize() const { return vertexSize_; }
        /// Return vertex elements
        const PODVector<VertexElement>& GetElements() const { return elements_; }
        /// Return vertex element, or null if does not exist
        const VertexElement* GetElement(VertexElementSemantic semantic, unsigned char index = 0) const;
        /// Return vertex element with specific type, or null if does not exist
        const VertexElement* GetElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0) const;
        /// Return whether has a specified element semantic
        bool HasElement(VertexElementSemantic semantic, unsigned char index = 0) const { return GetElement(semantic, index) != nullptr; }
        /// Return whether has an element semantic with specific type.
        bool HasElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0) const { return GetElement(type, semantic, index) != nullptr; }
        /// Return offset of an element within vertex
        unsigned GetElementOffset(VertexElementSemantic semantic, unsigned char index = 0) const;
        /// Return offset of an element with specific type within vertex
        unsigned GetElementOffset(VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0);
        /// Return CPU memory shadow data
        unsigned char* GetShadowData() const { return shadowData_.Get(); }
        /// Return shared array pointer to the CPU memory shadow data
        SharedArrayPtr<unsigned char> GetShadowDataShared() const { return shadowData_; }
        /// Return buffer hash for build vertex declarations. Used internally
        unsigned long long GetBufferHash(unsigned streamIndex) { return elementHash_ << (streamIndex * 16); }
        /// Return element with specified type and semantic from a vertex element list
        static const VertexElement* GetElement(const PODVector<VertexElement>& elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0);
        /// Return whether element list has a specified element type and semantic
        static bool HasElement(const PODVector<VertexElement>& elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0);
        /// Return element offset for specified type and semantic form a vertex element list
        static unsigned GetElementOffset(const PODVector<VertexElement>& elements, VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0);
        /// Return vertex size from an elements list
        static unsigned GetVertexSize(const PODVector<VertexElement>& elements);
        /// Update offsets of vertex elements
        static void UpdateOffsets(PODVector<VertexElement>& elements);

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
        /// Buffer locking state
        LockState lockState_{LOCK_NONE};
        /// Lock start vertex
        unsigned lockStart_;
        /// Lock number of vertices
        unsigned lockCount_;
        /// Scratch buffer for fallback locking.
        void* lockScratchData_{};
        /// Dynamic flag
        bool dynamic_{};
        /// Shadowed flag
        bool shadowed_{};
        /// Discard lock flag (OpenGL only).
        bool discardLock_{};
    };
}
