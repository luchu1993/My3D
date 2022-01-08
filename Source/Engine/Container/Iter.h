//
// Created by luchu on 2022/1/2.
//

#pragma once

namespace My3D
{

template <typename T> struct RandomAccessIterator
{
    RandomAccessIterator() : ptr_(nullptr) { }

    explicit RandomAccessIterator(T* ptr) : ptr_(ptr) { }

    T* operator->() const { return ptr_; }

    T& operator*() const { return *ptr_; }

    RandomAccessIterator<T>& operator++()
    {
        ++ptr_;
        return *this;
    }

    RandomAccessIterator<T> operator++(int)
    {
        RandomAccessIterator<T> it = *this;
        ++ptr_;
        return it;
    }

    RandomAccessIterator<T>& operator--()
    {
        --ptr_;
        return *this;
    }

    RandomAccessIterator<T> operator--(int)
    {
        RandomAccessIterator<T> it = *this;
        --ptr_;
        return it;
    }

    RandomAccessIterator<T>& operator +=(int value)
    {
        ptr_ += value;
        return *this;
    }

    RandomAccessIterator<T>& operator -=(int value)
    {
        ptr_ -= value;
        return *this;
    }

    RandomAccessIterator<T> operator +(int value) const { return RandomAccessIterator<T>(ptr_ + value); }

    RandomAccessIterator<T> operator -(int value) const { return RandomAccessIterator<T>(ptr_ - value); }

    int operator -(const RandomAccessIterator& rhs) const { return (int) (ptr_ - rhs.ptr_); }

    bool operator ==(const RandomAccessIterator& rhs) const { return ptr_ == rhs.ptr_; }

    bool operator !=(const RandomAccessIterator& rhs) const { return ptr_ != rhs.ptr_; }

    bool operator <(const RandomAccessIterator& rhs) const { return ptr_ < rhs.ptr_; }

    bool operator >(const RandomAccessIterator& rhs) const { return ptr_ > rhs.ptr_; }

    bool operator <=(const RandomAccessIterator& rhs) const { return ptr_ <= rhs.ptr_; }

    bool operator >=(const RandomAccessIterator& rhs) const { return ptr_ >= rhs.ptr_; }

    T* ptr_;
};


template <typename T> struct RandomAccessConstIterator
{
    RandomAccessConstIterator() : ptr_(nullptr) { }
    explicit RandomAccessConstIterator(const T* ptr) : ptr_(ptr) { }
    RandomAccessConstIterator(const RandomAccessIterator<T>& rhs) : ptr_(rhs.ptr_) { }
    RandomAccessConstIterator<T>& operator=(const RandomAccessIterator<T>& rhs)
    {
        ptr_ = rhs.ptr_;
        return *this;
    }

    const T* operator ->() const { return ptr_; }

    const T& operator *() const { return  *ptr_; }

    RandomAccessConstIterator<T>& operator++()
    {
        ++ptr_;
        return *this;
    }

    RandomAccessConstIterator<T> operator++(int)
    {
        RandomAccessConstIterator<T> it = this;
        ++ptr_;
        return it;
    }

    RandomAccessConstIterator<T>& operator--()
    {
        --ptr_;
        return *this;
    }

    RandomAccessConstIterator<T> operator--(int)
    {
        RandomAccessConstIterator<T> it = this;
        --ptr_;
        return it;
    }

    RandomAccessConstIterator<T>& operator+=(int value)
    {
        ptr_ += value;
        return *this;
    }

    RandomAccessConstIterator<T>& operator-=(int value)
    {
        ptr_ -= value;
        return *this;
    }

    RandomAccessConstIterator<T> operator+(int value) { return RandomAccessConstIterator<T>(ptr_ + value); }

    RandomAccessConstIterator<T> operator-(int value) { return RandomAccessConstIterator<T>(ptr_ - value); }

    int operator -(const RandomAccessConstIterator<T>& rhs) { return (int)(ptr_ - rhs.ptr_); }

    bool operator ==(const RandomAccessConstIterator<T>& rhs) { return ptr_ == rhs.ptr_; }

    bool operator !=(const RandomAccessConstIterator<T>& rhs) { return ptr_ != rhs.ptr_; }

    bool operator >(const RandomAccessConstIterator<T>& rhs) { return ptr_ > rhs.ptr_; }

    bool operator <(const RandomAccessConstIterator<T>& rhs) { return ptr_ < rhs.ptr_; }

    bool operator <=(const RandomAccessConstIterator<T>& rhs) { return ptr_ <= rhs.ptr_; }

    bool operator >=(const RandomAccessConstIterator<T>& rhs) { return ptr_ >= rhs.ptr_; }

    const T* ptr_;
};
}
