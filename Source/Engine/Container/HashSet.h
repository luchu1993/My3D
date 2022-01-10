//
// Created by luchu on 2022/1/10.
//

#pragma once

#include "My3D.h"
#include "Container/HashBase.h"

#include <cassert>


namespace My3D
{
    /// Hash set template class.
    template <typename T> class MY3D_API HashSet : public HashBase
    {
    public:
        /// Hash set node
        struct Node : public HashNodeBase
        {
            /// Construct
            Node() = default;
            /// Construct with key
            explicit Node(const T& key) : key_(key) { }
            /// Key
            T key_;
            /// Return next node
            Node* Next() const { return static_cast<Node*>(next_); }
            /// Return previous node.
            Node* Prev() const { return static_cast<Node*>(prev_); }
            /// Return next node in the bucket.
            Node* Down() const { return static_cast<Node*>(down_); }
        };
        /// Hash set node iterator
        struct Iterator
        {

        };
        /// Hash set node const iterator
        struct ConstIterator
        {

        };
        /// Construct
        HashSet()
        {
            allocator_ = AllocatorInitialize((unsigned) sizeof(Node));
        }
    };
}
