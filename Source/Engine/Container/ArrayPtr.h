//
// Created by luchu on 2022/1/21.
//

#pragma once

#include "Container/RefCounted.h"

#include <cassert>

namespace My3D
{
/// Shared array pointer template class. Uses non-intrusive reference counting.
template <typename T> class SharedArrayPtr
    {
    public:
        /// Construct a null shared array pointer
        SharedArrayPtr()
            : ptr_(nullptr)
            , refCount_(nullptr)
        {
        }
        /// Copy-construct from another shared array pointer.
        SharedArrayPtr(const SharedArrayPtr<T>& rhs)
            : ptr_(rhs.ptr_)
            , refCount_(rhs.refCount_)
        {
            AddRef();
        }
        /// Construct from a raw pointer.
        explicit SharedArrayPtr(T* ptr)
            : ptr_(ptr)
            , refCount_(new RefCount())
        {
            AddRef();
        }
        /// Destruct
        ~SharedArrayPtr()
        {
            ReleaseRef();
        }
        /// Assign from another shared array pointer.
        SharedArrayPtr<T>& operator =(const SharedArrayPtr<T>& rhs)
        {
            if (ptr_ == rhs.ptr_)
                return *this;
            ReleaseRef();
            ptr_ = rhs.ptr_;
            refCount_ = rhs.refCount_;
            AddRef();

            return *this;
        }
        /// Assign from a raw pointer.
        SharedArrayPtr<T>& operator =(T* ptr)
        {
            if (ptr_ == ptr)
                return *this;

            ReleaseRef();
            if (ptr)
            {
                ptr_ = ptr;
                refCount_ = new RefCount();
                AddRef();
            }

            return *this;
        }
        /// Point to the array.
        T* operator ->() const
        {
            assert(ptr_);
            return ptr_;
        }
        /// Dereference the array.
        T& operator *() const
        {
            assert(ptr_);
            return *ptr_;
        }
        /// Subscript the array.
        T& operator [](int index)
        {
            assert(ptr_);
            return ptr_[index];
        }
        /// Test for equality with another shared array pointer.
        bool operator ==(const SharedArrayPtr<T>& rhs) const { return ptr_ == rhs.ptr_; }
        /// Test for inequality with another shared array pointer.
        bool operator !=(const SharedArrayPtr<T>& rhs) const { return ptr_ != rhs.ptr_; }
        /// Test for less than with another array pointer.
        bool operator <(const SharedArrayPtr<T>& rhs) const { return ptr_ < rhs.ptr_; }
        /// Convert to a raw pointer.
        operator T*() const { return ptr_; }    // NOLINT(google-explicit-constructor)
        /// Reset to null and release the array reference.
        void Reset() { ReleaseRef(); }
        /// Perform a static cast from a shared array pointer of another type.
        template <class U> void StaticCast(const SharedArrayPtr<U>& rhs)
        {
            ReleaseRef();
            ptr_ = static_cast<T*>(rhs.Get());
            refCount_ = rhs.RefCountPtr();
            AddRef();
        }
        /// Perform a reinterpret cast from a shared array pointer of another type.
        template <class U> void ReinterpretCast(const SharedArrayPtr<U>& rhs)
        {
            ReleaseRef();
            ptr_ = reinterpret_cast<T*>(rhs.Get());
            refCount_ = rhs.RefCountPtr();
            AddRef();
        }
        /// Check if the pointer is null.
        bool Null() const { return ptr_ == nullptr; }
        /// Check if the pointer is not null.
        bool NotNull() const { return ptr_ != nullptr; }
        /// Return the raw pointer.
        T* Get() const { return ptr_; }
        /// Return the array's reference count, or 0 if the pointer is null.
        int Refs() const { return refCount_ ? refCount_->refs_ : 0; }
        /// Return the array's weak reference count, or 0 if the pointer is null.
        int WeakRefs() const { return refCount_ ? refCount_->weakRefs_ : 0; }
        /// Return pointer to the RefCount structure.
        RefCount* RefCountPtr() const { return refCount_; }
        /// Return hash value for HashSet & HashMap.
        unsigned ToHash() const { return (unsigned)((size_t)ptr_ / sizeof(T)); }

    private:
        /// Prevent direct assignment from a shared array pointer of different type.
        template<typename U> SharedArrayPtr<T>& operator=(const SharedArrayPtr<U>& rhs);
        /// Add a reference to the array pointed to
        void AddRef()
        {
            if (refCount_)
            {
                assert(refCount_->refs_ >= 0);
                ++(refCount_->refs_);
            }
        }
        //// Release the array reference and delete it and the RefCount structure if necessary.
        void ReleaseRef()
        {
            if (refCount_)
            {
                assert(refCount_->refs_ > 0);
                --(refCount_->refs_);
                if (!refCount_->refs_)
                {
                    refCount_->refs_ = -1;
                    delete[] ptr_;
                }

                if (refCount_->refs_ < 0 && !refCount_->weakRefs_)
                    delete refCount_;
            }

            ptr_ = nullptr;
            refCount_ = nullptr;
        }
        /// Pointer to the array
        T* ptr_;
        /// Pointer to the RefCount structure
        RefCount* refCount_;
    };
/// Perform a static cast from one shared array pointer type to another.
template <typename T, typename U> SharedArrayPtr<T> StaticCast(const SharedArrayPtr<U>& ptr)
{
    SharedArrayPtr<T> ret;
    ret.StaticCast(ptr);
    return ret;
}

/// Perform a reinterpret cast from one shared array pointer type to another.
template <typename T, typename U> SharedArrayPtr<T> ReinterpretCast(const SharedArrayPtr<U>& ptr)
{
    SharedArrayPtr<T> ret;
    ret.ReinterpretCast(ptr);
    return ret;
}

/// Weak array pointer template class. Uses non-intrusive reference counting.
template <typename T> class WeakArrayPtr
{

private:
    /// Pointer to the array.
    T* ptr_;
    /// Pointer to the RefCount structure.
    RefCount* refCount_;
};

}
