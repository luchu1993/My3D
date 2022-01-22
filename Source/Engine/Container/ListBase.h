//
// Created by luchu on 2022/1/22.
//

#pragma once

#include "Container/Allocator.h"
#include "Container/Swap.h"

namespace My3D
{
/// Doubly-linked list node base class.
struct ListNodeBase
{
    /// Construct
    ListNodeBase() : prev_(nullptr), next_(nullptr) {}

    /// Previous node
    ListNodeBase* prev_;
    /// Next node
    ListNodeBase* next_;
};

/// Doubly-linked list iterator base class.
struct ListIteratorBase
{
    /// Construct
    ListIteratorBase() : ptr_(nullptr) { }
    /// Construct with a node pointer
    explicit ListIteratorBase(ListNodeBase* ptr) : ptr_(ptr) { }
    /// Test for equality with another iterator
    bool operator ==(const ListIteratorBase& rhs) const { return ptr_ == rhs.ptr_; }
    /// Test for inequality with another iterator
    bool operator !=(const ListIteratorBase& rhs) const { return ptr_ != rhs.ptr_; }
    /// Go to the next node
    void GotoNext()
    {
        if (ptr_)
            ptr_ = ptr_->next_;
    }
    /// Go to the previous node
    void GotoPrev()
    {
        if (ptr_)
            ptr_ = ptr_->prev_;
    }

    /// Node pointer
    ListNodeBase* ptr_;
};

/// Doubly-linked list base class.
class MY3D_API ListBase
{
public:
    /// Construct
    ListBase() : head_(nullptr), tail_(nullptr), allocator_(nullptr), size_(0) { }

    /// Swap with another linked list
    void Swap(ListBase& rhs)
    {
        My3D::Swap(head_, rhs.head_);
        My3D::Swap(tail_, rhs.tail_);
        My3D::Swap(allocator_, rhs.allocator_);
        My3D::Swap(size_, rhs.size_);
    }

protected:
    /// Head node pointer
    ListNodeBase* head_;
    // Tail node pointer
    ListNodeBase* tail_;
    /// Node allocator
    AllocatorBlock* allocator_;
    /// Number of nodes
    unsigned size_;
};

}
