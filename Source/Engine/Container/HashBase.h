//
// Created by luchu on 2022/1/3.
//

#pragma once

#include "My3D.h"
#include "Container/Allocator.h"
#include "Container/Swap.h"


namespace My3D
{
    struct MY3D_API HashNodeBase
    {
        /// Construct
        HashNodeBase()
            : down_(nullptr)
            , prev_(nullptr)
            , next_(nullptr)
        {
        }
        /// Next node in the bucket
        HashNodeBase* down_;
        /// Previous node
        HashNodeBase* prev_;
        /// Next node
        HashNodeBase* next_;
    };

    struct MY3D_API HashIteratorBase
    {
        HashIteratorBase() : ptr_(nullptr) { }
        explicit HashIteratorBase(HashNodeBase* ptr) : ptr_(ptr) { }
        /// Test for equality
        bool operator ==(const HashIteratorBase& rhs) const { return rhs.ptr_ == ptr_; }
        /// Test for inequality
        bool operator !=(const HashIteratorBase& rhs) const { return rhs.ptr_ != ptr_; }
        /// Goto next node
        void GotoNext()
        {
            if (ptr_)
                ptr_ = ptr_->next_;
        }
        /// Goto previous node
        void GotoPrev()
        {
            if (ptr_)
                ptr_ = ptr_->prev_;
        }
        /// Node pointer
        HashNodeBase* ptr_;
    };

    class MY3D_API HashBase
    {
    public:
        /// Initial amount of buckets
        static const unsigned MIN_BUCKETS = 8;
        /// Maximum load factor
        static const unsigned MAX_LOAD_FACTOR = 4;
        /// Construct
        HashBase()
            : head_(nullptr)
            , tail_(nullptr)
            , ptrs_(nullptr)
            , allocator_(nullptr)
        {
        }
        /// Swap with another hash set or map
        void Swap(HashBase& rhs)
        {
            My3D::Swap(head_, rhs.head_);
            My3D::Swap(tail_, rhs.tail_);
            My3D::Swap(ptrs_, rhs.ptrs_);
            My3D::Swap(allocator_, rhs.allocator_);
        }
        /// Return number of elements
        unsigned Size() const { return ptrs_ ? (reinterpret_cast<unsigned*>(ptrs_))[0] : 0; }
        /// Return number of buckets
        unsigned NumBuckets() const { return ptrs_ ? (reinterpret_cast<unsigned*>(ptrs_))[1] : 0; }
        /// Return whether has no elements
        bool Empty() const { return Size() == 0; }
    protected:
        /// Allocate bucket head pointers
        void AllocateBuckets(unsigned size, unsigned numBuckets);
        /// Reset bucket head pointers
        void ResetPtrs();
        /// Set new size
        void SetSize(unsigned size) { if (ptrs_) (reinterpret_cast<unsigned*>(ptrs_))[0] = size; }
        /// Return bucket head pointers
        HashNodeBase** Ptrs() const { return ptrs_ ? ptrs_ + 2 : nullptr; }
        /// List head node pointer
        HashNodeBase* head_;
        /// List tail node pointer
        HashNodeBase* tail_;
        /// Bucket head pointers.
        HashNodeBase** ptrs_;
        /// Node allocator
        AllocatorBlock* allocator_;
    };
}