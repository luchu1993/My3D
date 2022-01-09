//
// Created by luchu on 2022/1/2.
//

#pragma once

#include "My3D.h"
#include "Container/RefCounted.h"
#include "Container/Swap.h"

#include <cstddef>
#include <utility>
#include <cassert>


namespace My3D
{

template <typename T>
class SharedPtr
{
public:
    /// Construct a null shared pointer
    SharedPtr() noexcept : ptr_(nullptr) { }
    /// Construct a null shared pointer
    explicit SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr) { }
    /// Construct from a raw pointer
    explicit SharedPtr(T* ptr) noexcept: ptr_(ptr) { AddRef(); }
    /// Copy-construct from another shared pointer
    SharedPtr(const SharedPtr<T>& rhs) noexcept : ptr_(rhs.ptr_) { AddRef(); }
    /// Move-construct from another shared pointer
    SharedPtr(SharedPtr<T>&& rhs) noexcept : ptr_(rhs.ptr_) { rhs.ptr_ = nullptr; }
    /// Copy-construct from another shared pointer allowing implicit upcasting
    template<typename U> SharedPtr(const SharedPtr<U>& rhs) noexcept : ptr_(rhs.ptr_) { AddRef(); }
    /// Destruct. Release the object reference
    ~SharedPtr() noexcept { ReleaseRef(); }
    /// Assign from another shared pointer
    SharedPtr<T>& operator=(const SharedPtr<T>& rhs)
    {
        if (ptr_ == rhs.ptr_)
            return *this;

        SharedPtr<T> copy(rhs);
        Swap(copy);

        return *this;
    }
    /// Move-assign form another shared pointer
    SharedPtr<T>& operator=(SharedPtr<T>&& rhs) noexcept
    {
        SharedPtr<T> copy(std::move(rhs));
        Swap(copy);

        return *this;
    }
    /// Assign from another shared pointer allowing implicit upcasting
    template<typename U> SharedPtr<T>& operator=(const SharedPtr<U>& rhs)
    {
        if (ptr_ == rhs.ptr_)
            return *this;

        SharedPtr<T> copy(this);
    }
    /// Assign from a raw pointer
    SharedPtr<T>& operator=(T* ptr)
    {
        if (ptr_ == ptr)
            return *this;

        SharedPtr<T> copy(ptr);
        Swap(copy);

        return *this;
    }
    /// Point to the object
    T* operator->() const
    {
        assert(ptr_ != nullptr);
        return ptr_;
    }
    /// Dereference the object
    T& operator*() const
    {
        assert(ptr_ != nullptr);
        return ptr_;
    }
    /// Subscript the object if applicable
    T& operator[](int index)
    {
        assert(ptr_ != nullptr);
        return ptr_[index];
    }
    /// Test for less than with another shared pointer
    template<typename U> bool operator <(const SharedPtr<U>& rhs) { return ptr_ < rhs.ptr_; }
    /// Test for equality with another shared pointer
    template<typename U> bool operator ==(const SharedPtr<U>& rhs) { return ptr_ == rhs.ptr_; }
    /// Test for inequality with another shared pointer
    template<typename U> bool operator !=(const SharedPtr<U>& rhs) { return ptr_ != rhs.ptr_; }
    /// Convert to a raw pointer
    operator T*() const { return ptr_; }
    /// Swap with another SharedPtr
    void Swap(SharedPtr<T>& rhs) { My3D::Swap(ptr_, rhs.ptr_); }
    /// Reset with another pointer
    void Reset(T* ptr = nullptr)
    {
        SharedPtr<T> copy(ptr);
        Swap(copy);
    }
    /// Detach without destroying the object even if the refcount goes zero.
    T* Detach()
    {
        T* ptr = ptr_;
        if (ptr)
        {
            RefCount* refCount = RefCountPtr();
            ++refCount->refs_;
            Reset();
            --refCount->refs_;
        }

        return ptr;
    }
    /// Perform a static cast from a shared pointer of another type
    template <class U> void StaticCast(const SharedPtr<U>& rhs)
    {
        SharedPtr<T> copy(static_cast<T*>(rhs.Get()));
        Swap(copy);
    }
    /// Perform a dynamic cast from a shared pointer of another type.
    template <typename U> void DynamicCast(const SharedPtr<U>& rhs)
    {
        SharedPtr<T> copy(dynamic_cast<T*>(rhs.Get()));
        Swap(copy);
    }
    /// Check if the pointer is null
    bool Null() const { return ptr_ == nullptr; }
    /// Check if the pointer is not null
    bool NotNull() const { return ptr_ != nullptr; }
    /// Return the raw pointer
    T* Get() const { return ptr_; }
    /// Return the object reference count
    int Refs() const { return ptr_ ? ptr_->Refs() : 0; }
    /// Return the object weak reference count
    int WeakRefs() const { return ptr_ ? ptr_->WeakRefs() : 0; }
    /// Return pointer to the RefCount
    RefCount* RefCountPtr() const { return ptr_ ? ptr_->RefCountPtr() : nullptr; }
    /// Return pointer to the HashSet & HashMap
    unsigned ToHash() const { return (unsigned)((size_t)ptr_) / sizeof(T);}
private:
    template<typename U> friend class SharedPtr;
    /// Add a reference to the object pointed to.
    void AddRef()
    {
        if (ptr_) ptr_->AddRef();
    }
    /// Release the object reference and delete it if necessary
    void ReleaseRef()
    {
        if (ptr_)
        {
            ptr_->ReleaseRef();
            ptr_ = nullptr;
        }
    }
    /// Pointer to the object
    T* ptr_;
};

/// Perform a static cast from one shared pointer type to another
template <typename T, typename U>
SharedPtr<T> StaticCast(const SharedPtr<U>& ptr)
{
    SharedPtr<T> ret;
    ret.StaticCast(ptr);
    return ret;
}
/// Perform a dynamic cast from one shared pointer type to another
template <typename T, typename U>
SharedPtr<T> DynamicCast(const SharedPtr<U>& ptr)
{
    SharedPtr<T> ret;
    ret.DynamicCast(ptr);
    return ret;
}

/// Weak pointer template class with intrusive reference counting. Dose not keep the object pointed to alive.
template <typename T>
class MY3D_API WeakPtr
{
public:
    /// Construct a null weak pointer
    WeakPtr() noexcept : ptr_(nullptr), refCount_(nullptr) { }
    /// Construct a null weak pointer
    WeakPtr(std::nullptr_t) noexcept : ptr_(nullptr), refCount_(nullptr) { }
    /// Copy-construct from another weak pointer
    WeakPtr(const WeakPtr<T>& rhs) noexcept : ptr_(rhs.ptr_), refCount_(rhs.refCount_) { AddRef(); }
    /// Move-construct from another weak pointer
    WeakPtr(WeakPtr<T>&& rhs) noexcept : ptr_(rhs.ptr_), refCount_(rhs.refCount_)
    {
        rhs.ptr_ = nullptr;
        rhs.refCount_ = nullptr;
    }
    /// Copy-construct from another weak pointer allowing implicit upcasting
    template<typename U> WeakPtr(const WeakPtr<U>& rhs) noexcept : ptr_(rhs.ptr_), refCount_(rhs.refCount_)
    {
        AddRef();
    }
    /// Construct from a shared pointer
    WeakPtr(const SharedPtr<T>& rhs) noexcept : ptr_(rhs.Get()), refCount_(rhs.RefCountPtr())
    {
        AddRef();
    }
    /// Construct from a raw pointer
    explicit WeakPtr(T* ptr) noexcept : ptr_(ptr), refCount_(ptr ? ptr->RefCountPtr() : nullptr)
    {
        AddRef();
    }
    /// Destruct. Release the weak reference to the object.
    ~WeakPtr() noexcept
    {
        ReleaseRef();
    }
    /// Assign from a shared pointer
    WeakPtr<T>& operator =(const SharedPtr<T>& rhs)
    {
        if (ptr_ == rhs.Get() && refCount_ == rhs.RefCountPtr())
            return *this;

        WeakPtr<T> copy(rhs);
        Swap(copy);

        return *this;
    }
    /// Assign from a weak pointer.
    WeakPtr<T>& operator =(const WeakPtr<T>& rhs)
    {
        if (ptr_ == rhs.ptr_ && refCount_ == rhs.refCount_)
            return *this;

        WeakPtr<T> copy(rhs);
        Swap(copy);

        return *this;
    }
    /// Move-assign from another weak pointer.
    WeakPtr<T>& operator =(WeakPtr<T>&& rhs) noexcept
    {
        WeakPtr<T> copy(std::move(rhs));
        Swap(copy);

        return *this;
    }
    /// Assign from a raw pointer
    WeakPtr<T>& operator =(T* ptr)
    {
        RefCount* refCount = ptr ? ptr_->RefCountPtr() : nullptr;
        if (ptr_ == ptr && refCount_ == refCount)
            return *this;

        ReleaseRef();
        ptr_ = ptr;
        refCount = refCount;
        AddRef();

        return *this;
    }
    /// Convert to a raw pointer, null if the object is expired.
    operator T*() const { return Get(); }   // NOLINT(google-explicit-constructor)
    SharedPtr<T> Lock() const
    {
        if (Expired())
            return SharedPtr<T>();
        else
            return SharedPtr<T>(ptr_);
    }

    T* Get() const
    {
        return Expired() ? nullptr : ptr_;
    }

    void Swap(WeakPtr<T>& rhs)
    {
        My3D::Swap(ptr_, rhs.ptr_);
        My3D::Swap(refCount_, rhs.refCount_);
    }

    void Reset(T* ptr = nullptr)
    {
        WeakPtr<T> copy(ptr);
        Swap(copy);
    }
    /// Return whether the object has expired. If null pointer, always return true.
    bool Expired() const { return refCount_ ? refCount_->refs_ < 0 : true; }
    /// Return hash value for HashSet & HashMap
    unsigned ToHash() const { return (unsigned)((size_t)ptr_ / sizeof (T)); }

private:
    template<typename U> friend class WeakPtr;
    void AddRef()
    {
        if (refCount_)
        {
            assert(refCount_->weakRefs_ >= 0);
            ++(refCount_->weakRefs_);
        }
    }

    void ReleaseRef()
    {
        if (refCount_)
        {
            assert(refCount_->weakRefs_ > 0);
            --(refCount_->weakRefs_);

            if (Expired() && !refCount_->weakRefs_)
                delete refCount_;
        }

        ptr_ = nullptr;
        refCount_ = nullptr;
    }

    T* ptr_;
    ///Pointer to the RefCount
    RefCount* refCount_;
};

template <typename T>
class MY3D_API UniquePtr
{

};

}
