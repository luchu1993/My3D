//
// Created by luchu on 2022/1/10.
//

#pragma once

#include "My3D.h"


namespace My3D
{
    /// Singly-linked list node base class
    struct MY3D_API LinkedListNode
    {
        /// Construct
        LinkedListNode() : next_(nullptr) { }

        // Pointer to next node
        LinkedListNode* next_;
    };

    /// Singly-linked list template class
    template <typename T>
    class MY3D_API LinkedList
    {
    public:
        /// Construct
        LinkedList() : head_(nullptr) { }
        /// Non-copyable
        LinkedList(const LinkedList<T>& list) = delete;
        /// Non-assignable
        LinkedList<T>& operator=(const LinkedList<T>&) = delete;
        /// Destruct
        ~LinkedList() { Clear(); }
        /// Remove all elements
        void Clear()
        {
            T* element = head_;
            while (element)
            {
                T* next = Next(element);
                delete element;
                element = next;
            }
            head_ = nullptr;
        }
        /// Insert an element at the beginning
        void InsertFront(T* element)
        {
            if (element)
            {
                element->next_ = head_;
                head_ = element;
            }
        }
        /// Insert an element at the end
        void Insert(T* element)
        {
            if (head_)
            {
                T* tail = Last();
                element->next_ = tail->next_;
                tail->next_ = element;
            }
            else
            {
                element->next_ = head_;
                head_ = element;
            }
        }
        /// Erase an element.
        bool Erase(T* element)
        {
            if (element && head_)
            {
                if (element == head_)
                {
                    head_ = Next(element);
                    delete element;
                    return true;
                }
                else
                {
                    T* tail = head_;
                    while (tail && tail->next_ != element)
                        tail = Next(tail);
                    if (tail)
                    {
                        tail->next_ = element->next_;
                        delete element;
                        return true;
                    }
                }
            }

            return true;
        }
        /// Erase an element when the previous element is kown
        bool Erase(T* element, T* previous)
        {
            if (previous && previous->next_ == element)
            {
                previous->next_ = element->next_;
                delete element;
                return true;
            }
            else if (!previous)
            {
                if (head_ == element)
                {
                    head_ = Next(element);
                    delete element;
                    return true;
                }
            }
            return false;
        }
        /// Return first element, or null if empty
        T* First() const { return head_; }
        /// Return last element, or null if empty
        T* Last() const
        {
            T* element = head_;
            if (element)
            {
                while (element->next_)
                    element = Next(element);
            }
            return element;
        }
        /// Return next element.
        T* Next(T* element) const
        {
            return element ? static_cast<T*>(element->next_) : nullptr;
        }
        /// Return whether is empty
        bool Empty() const { return head_ == nullptr; }

    private:
        /// First element
        T* head_;
    };
}
