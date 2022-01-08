//
// Created by luchu on 2022/1/3.
//

#pragma once

#include "My3D.h"
#include <utility>


namespace My3D
{

    // struct AllocatorBlock;
    struct AllocatorNode;

    /// Allocator memory block
    struct MY3D_API AllocatorBlock
    {
        /// Size of node
        unsigned nodeSize_;
        /// Number of nodes in this block
        unsigned capacity_;
        /// First free node
        AllocatorNode* free_;
        /// Next allocator block
        AllocatorBlock* next_;
    };

    /// Allocator node
    struct MY3D_API AllocatorNode
    {
        /// Next free node
        AllocatorNode* next_;
    };

    /// Initialize a fixed-size allocator with the node size and initial capacity
    MY3D_API AllocatorBlock* AllocatorInitialize(unsigned nodeSize, unsigned initialCapacity = 1);
    /// Uninitialize a fixed-size allocator. Frees all blocks in the chain
    MY3D_API void AllocatorUninitialize(AllocatorBlock* allocator);
    /// Reserve a node.
    MY3D_API void* AllocatorReserve(AllocatorBlock* allocator);
    /// Free a node. Does not free any blocks
    MY3D_API void AllocatorFree(AllocatorBlock* allocator, void* ptr);

    template <typename T> class MY3D_API Allocator
    {
    public:
        /// Construct
        explicit Allocator(unsigned initialCapacity = 0)
            : allocator_(nullptr)
        {
            if (initialCapacity)
                allocator_ = AllocatorInitialize((unsigned)sizeof(T), initialCapacity);
        }
        /// Destruct
        ~Allocator()
        {
            AllocatorUninitialize(allocator_);
        }
        /// Prevent copy construction
        Allocator(const Allocator<T>& rhs) = delete;
        /// Prevent assigment
        Allocator<T>& operator=(const Allocator<T>& rhs) = delete;
        /// Reverse and default-construct an object
        T* Reverse()
        {
            if (!allocator_)
                allocator_ = AllocatorInitialize((unsigned) sizeof(T));
            auto* newObject = static_cast<T*>(AllocatorReserve(allocator_));
            new (newObject) T();

            return newObject;
        }
        /// Reserve and copy-construct an object
        T* Reserve(const T& object)
        {
            if (!allocator_)
                allocator_ = AllocatorInitialize((unsigned) sizeof(T));
            auto* newObject = static_cast<T*>(AllocatorReserve(allocator_));
            new (newObject) T(object);

            return newObject;
        }
        /// Reserve and create an object
        template<typename... Args>
        T* Reserve(Args&&... args)
        {
            if (!allocator_)
                allocator_ = AllocatorInitialize((unsigned) sizeof(T));
            auto* newObject = static_cast<T*>(AllocatorReserve(allocator_));
            new (newObject) T(std::forward<Args>(args)...);

            return newObject;
        }
        /// Destruct and free an object
        void Free(T* object)
        {
            (object)->~T();
            AllocatorFree(allocator_, object);
        }
    private:
        /// Allocator block
        AllocatorBlock* allocator_;
    };
}

