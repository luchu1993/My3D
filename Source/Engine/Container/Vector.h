//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "Container/VectorBase.h"
#include "Container/Iter.h"

#include <cassert>
#include <utility>
#include <algorithm>


namespace My3D
{

template <typename T> class Vector : public VectorBase
{
    struct CopyTag {};
    struct MoveTag {};

public:
    using ValueType = T;
    using Iterator = RandomAccessIterator<T>;
    using ConstIterator = RandomAccessConstIterator<T>;

    Vector() noexcept = default;
    explicit Vector(unsigned size)
    {
        Resize(size);
    }
    /// Construct with initial size and default value
    Vector(unsigned size, const T& value)
    {
        Resize(size);
        for (unsigned i = 0; i < size; ++i)
            At(i) = value;
    }
    /// Construct with initial data
    Vector(const T* data, unsigned size)
    {
        DoInsertElements(0, data, data + size, CopyTag{});
    }
    /// Copy-construct from another vector
    Vector(const Vector<T>& vector)
    {
        DoInsertElements(0, vector.Begin(), vector.End(), CopyTag{});
    }
    /// Copy-construct from another vector by iterator
    Vector(const ConstIterator& start, const ConstIterator& end)
    {
        DoInsertElements(0, start, end, CopyTag{});
    }
    /// Move-construct from another vector
    Vector(Vector<T>&& vector) noexcept
    {
        Swap(vector);
    }
    /// Aggregate initialization constructor
    Vector(const std::initializer_list<T>& list)
    {
        for (auto it = list.begin(); it != list.end(); ++it)
        {
            Push(*it);
        }
    }
    /// Destruct
    ~Vector()
    {
        DestructElements(Buffer(), size_);
        delete[] buffer_;
    }
    /// Assign from another vector
    Vector<T>& operator =(const Vector<T>& rhs)
    {
        if (&rhs != this)
        {
            Vector<T> copy(rhs);
            Swap(copy);
        }
        return *this;
    }
    /// Add an element
    Vector<T> operator+(const T& value)
    {
        Vector<T> ret(*this);
        ret.Push(value);
        return ret;
    }
    /// Add another vector
    Vector<T> operator+(const Vector<T>& rhs) const
    {
        Vector<T> ret(*this);
        ret.Push(rhs);
        return ret;
    }
    /// Test for equality with another vector
    bool operator ==(const Vector<T>& rhs) const
    {
        if (rhs.size_ != size_)
            return false;
        T* buffer = Buffer();
        T* rhsBuffer = rhs.Buffer();
        for (unsigned i = 0; i < size_; ++i)
        {
            if (buffer[i] != rhsBuffer[i])
                return false;
        }
        return true;
    }
    /// Test for inequality with another vector
    bool operator !=(const Vector<T>& rhs) const
    {
        if (rhs.size_ != size_)
            return true;
        T* buffer = Buffer();
        T* rhsBuffer = rhs.Buffer();
        for (unsigned i = 0; i < size_; ++i)
        {
            if (buffer[i] != rhsBuffer[i])
                return true;
        }
        return false;
    }
    /// Return element at index
    T& operator[](unsigned index)
    {
        assert(index < size_);
        return Buffer()[index];
    }

    const T& operator[](unsigned index) const
    {
        assert(index < size_);
        return Buffer()[index];
    }
    ///Return element at index
    T& At(unsigned index)
    {
        assert(index < size_);
        return Buffer()[index];
    }
    ///Return element at index
    const T& At(unsigned index) const
    {
        assert(index < size_);
        return Buffer()[index];
    }
    /// Create an element at the end
    template<typename... Args>
    T& EmplaceBack(Args&&... args)
    {
        if (size_ < capacity_)
        {
            ++size_;
            new (&Back()) T(std::forward<Args>(args)...);
        }
        else
        {
            T value(std::forward<Args>(args)...);
            Push(std::move(value));
        }
    }
    /// Add an element at the end
    void Push(const T& value)
    {
        if (size_ < capacity_)
        {
            ++size_;
            new (&Back()) T(value);
        }
        else
            DoInsertElements(size_, &value, &value + 1, CopyTag{});
    }
    /// Move-add an element at the end
    void Push(T&& value)
    {
        if (size_ < capacity_)
        {
            ++size_;
            new (&Back()) T(std::move(value));
        }
        else
            DoInsertElements(size_, &value, &value + 1, MoveTag{});
    }
    /// Add another vector at the end
    void Push(const Vector<T>& vector)
    {
        DoInsertElements(size_, vector.Begin(), vector.End(), CopyTag{});
    }
    /// Remove the last element
    void Pop()
    {
        if (size_) Resize(size_ - 1);
    }
    /// Insert an element at position
    void Insert(unsigned pos, const T& value)
    {
        DoInsertElements(pos, &value, &value + 1, CopyTag{});
    }
    /// Move-insert an element at position
    void Insert(unsigned pos, T&& value)
    {
        DoInsertElements(pos, &value, &value + 1, MoveTag{});
    }
    /// Insert another vector at position
    void Insert(unsigned pos, const Vector<T>& vector)
    {
        DoInsertElements(pos, vector.Begin(), vector.End(), CopyTag{});
    }
    /// Insert an element by iterator
    Iterator Insert(const Iterator& dest, const T& value)
    {
        auto pos = (unsigned) (dest - Begin());
        return DoInsertElements(pos, &value, &value + 1, CopyTag{});
    }
    /// Move-insert an element by iterator
    Iterator Insert(const Iterator& dest, T&& value)
    {
        auto pos = (unsigned) (dest - Begin());
        return DoInsertElements(pos, &value, &value + 1, MoveTag{});
    }
    /// Insert a vector by iterator
    Iterator Insert(const Iterator& dest, const Vector<T>& vector)
    {
        auto pos = (unsigned) (dest - Begin());
        return DoInsertElements(pos, vector.Begin(), vector.End(), CopyTag{});
    }
    /// Insert a vector partially by iterators
    Iterator Insert(const Iterator& dest, const ConstIterator& start, const ConstIterator& end)
    {
        auto pos = (unsigned) (dest - Begin());
        return DoInsertElements(pos, start, end, CopyTag{});
    }
    /// Insert elements
    Iterator Insert(const Iterator& dest, const T* start, const T* end)
    {
        auto pos = (unsigned) (dest - Begin());
        return DoInsertElements(pos, start, end, CopyTag{});
    }
    /// Erase a range of elements.
    void Erase(unsigned pos, unsigned length = 1)
    {
        if (pos + length > size_ || !length)
            return;
        DoEraseElements(pos, length);
    }
    /// Erase an element by iterator.
    Iterator Erase(const Iterator& it)
    {
        auto pos = (unsigned) (it - Begin());
        Erase(pos);
        return Begin() + pos;
    }
    /// Erase a range of elements by swapping elements from the end of the array.
    void EraseSwap(unsigned pos, unsigned length = 1)
    {
        unsigned shiftStartIndex = pos + length;
        if (shiftStartIndex > size_ || !length)
            return;

        unsigned newSize = size_ - length;
        unsigned trailingCount = size_ - shiftStartIndex;
        if (trailingCount < length)
        {
            DoEraseElements(pos, length);
        }
        else
        {
            T* buffer = Buffer();
            std::move(buffer + newSize, buffer + size_, buffer + pos);
            Resize(newSize);
        }
    }
    /// Erase an element by value. Return true if was found and erased.
    bool Remove(const T& value)
    {
        Iterator i = Find(value);
        if (i != End())
        {
            Erase(i);
            return true;
        }

        return false;
    }
    /// Erase an element by value by swapping with the last element.
    bool RemoveSwap(const T& value)
    {
        Iterator i = Find(value);
        if (i != End())
        {
            EraseSwap(i - Begin());
            return true;
        }

        return false;
    }
    /// Clear the vector
    void Clear() { Resize(0); }
    /// Resize the vector
    void Resize(unsigned newSize)
    {
        DoResize(newSize);
    }
    /// Resize the vector and fill new elements with default value
    void Resize(unsigned newSize, const T& value)
    {
        unsigned oldSize = Size();
        DoResize(newSize);
        for (unsigned i = oldSize; i < newSize; ++i)
        {
            At(i) = value;
        }
    }
    /// Set new capacity
    void Reserve(unsigned newCapacity)
    {
        if (newCapacity < size_)
            newCapacity = size_;

        if (newCapacity != capacity_)
        {
            T* newBuffer = nullptr;
            capacity_ = newCapacity;

            if (capacity_)
            {
                newBuffer = reinterpret_cast<T*>(AllocateBuffer((unsigned )(capacity_ * sizeof(T))));
                ConstructElements(newBuffer, Begin(), End(), MoveTag{});
            }
            // Delete the old buffer
            DestructElements(Buffer(), size_);
            delete[] buffer_;
            buffer_ = reinterpret_cast<unsigned char*>(newBuffer);
        }
    }
    /// Reallocate so that no extra memory is used
    void Compact() { Reserve(size_); }
    Iterator Find(const T& value)
    {
        Iterator it = Begin();
        while (it != End() && *it != value)
            ++it;
        return it;
    }

    ConstIterator Find(const T& value) const
    {
        ConstIterator it = Begin();
        while (it != End() && *it != value)
            ++it;
        return it;
    }

    unsigned IndexOf(const T& value) const
    {
        return Find(value) - Begin();
    }

    bool Contains(const T& value) const { return Find(value) != End(); }

    Iterator Begin() { return Iterator(Buffer()); }

    ConstIterator Begin() const { return ConstIterator(Buffer()); }

    Iterator End() { return Iterator(Buffer() + size_); }

    ConstIterator End() const { return ConstIterator(Buffer() + size_); }

    T& Front()
    {
        assert(size_);
        return Buffer()[0];
    }

    const T& Front() const
    {
        assert(size_);
        return Buffer()[0];
    }

    T& Back()
    {
        assert(size_);
        return Buffer()[size_ - 1];
    }

    const T& Back() const
    {
        assert(size_);
        return Buffer()[size_ - 1];
    }
    /// Return size of vector
    unsigned Size() const { return size_; }
    /// Return capacity of vector
    unsigned Capacity() const { return capacity_; }
    /// Return whether vector is empty
    bool Empty() const { return size_ == 0; }
    /// Return the buffer with right type
    T* Buffer() const { return reinterpret_cast<T*>(buffer_); }
private:
    /// Construct elements using default ctor
    static void ConstructElements(T* dest, unsigned count)
    {
        for (unsigned  i = 0; i < count; ++i)
            new (dest + i) T();
    }
    /// Copy-construct elements
    template<typename RandomIterT>
    static void ConstructElements(T* dest, RandomIterT start, RandomIterT end, CopyTag)
    {
        const auto count = (unsigned)(end - start);
        for (unsigned i = 0; i < count; ++i)
            new (dest + i) T(*(start + i));
    }
    /// Move-construct elements
    template<typename RandomIterT>
    static void ConstructElements(T* dest, RandomIterT start, RandomIterT end, MoveTag)
    {
        const auto count = (unsigned)(end - start);
        for (unsigned i = 0; i < count; ++i)
            new (dest + i) T(std::move(*(start + i)));
    }
    /// Calculate new vector capacity
    static unsigned CalculateCapacity(unsigned size, unsigned capacity)
    {
        if (!capacity)
            return size;
        else
        {
            while (capacity < size)
                capacity += (capacity + 1) >> 1;
            return capacity;
        }
    }
    /// Resize the vector and create/remove new elements as necessary
    void DoResize(unsigned newSize)
    {
        if (newSize < size_)
            DestructElements(Buffer() + newSize, size_ - newSize);
        else
        {
            if (newSize > capacity_)
            {
                T* src = Buffer();
                // Reallocate vector
                Vector<T> newVector;
                newVector.Reserve(CalculateCapacity(newSize, capacity_));
                newVector.size_ = size_;
                T* dest = newVector.Buffer();
                // Move old elements
                ConstructElements(dest, src, src + size_, MoveTag{});
                Swap(newVector);
            }
            // Initialize the new elements
            ConstructElements(Buffer() + size_, newSize - size_);
        }

        size_ = newSize;
    }

    template<typename Tag, class RandomIteratorT>
    Iterator DoInsertElements(unsigned pos, RandomIteratorT start, RandomIteratorT end, Tag)
    {
        if (pos > size_)
            pos = size_;

        const auto numElements = (unsigned)(end - start);
        if (size_ + numElements > capacity_)
        {
            T* src = Buffer();
            // Reallocate vector
            Vector<T> newVector;
            newVector.Reserve(CalculateCapacity(size_ + numElements, capacity_));
            newVector.size_ = size_ + numElements;
            T* dest = newVector.Buffer();
            // Copy or move new elements
            ConstructElements(dest + pos, start, end, Tag{});
            if (pos > 0)
                ConstructElements(dest, src, src + pos, MoveTag{});
            if (pos < size_)
                ConstructElements(dest + pos + numElements, src + pos, src + size_, MoveTag{});
            Swap(newVector);
        }
        else if (numElements > 0)
        {
            T* buffer = Buffer();
            // Copy or move new elements
            ConstructElements(buffer + size_, start, end, Tag{});
            // Rotate buffer
            if (pos < size_)
            {
                std::rotate(buffer + pos, buffer + size_,  buffer + size_ + numElements);
            }
            // Update size
            size_ += numElements;
        }

        return Begin() + pos;
    }
    /// Erase elements from the vector
    Iterator DoEraseElements(unsigned pos, unsigned count)
    {
        assert(count > 0);
        assert(pos + count <= size_);
        T* buffer = Buffer();
        std::move(buffer + pos + count, buffer + size_, buffer + pos);
        Resize(size_ - count);
        return Begin() + pos;
    }
    /// Call elements' destructors
    static void DestructElements(T* dest, unsigned count)
    {
        while (count--)
        {
            dest->~T();
            ++dest;
        }
    }
};


template <typename T> class PODVector : public VectorBase
{
public:
    using VarType = T;
    using Iterator = RandomAccessIterator<T>;
    using ConstIterator = RandomAccessConstIterator<T>;

    /// Construct empty
    PODVector() noexcept = default;
    /// Construct with initial size
    explicit PODVector(unsigned size)
    {
        Resize(size);
    }
    /// Constuct with initial size and default value
    PODVector(unsigned size, const T& value)
    {
        Resize(size);
        for (unsigned i = 0; i < size; ++i)
            At(i) = value;
    }
    /// Construct with initial data
    PODVector(const T* data, unsigned size)
    {
        Resize(size);
        CopyElements(Buffer(), data, size);
    }
    /// Construct from another vector
    PODVector(const PODVector<T>& vector)
    {
        *this = vector;
    }
    /// Aggregate initialization constructor
    PODVector(const std::initializer_list<T>& list) : PODVector()
    {
        for (const auto& elem : list)
        {
            Push(elem);
        }
    }
    /// Destruct
    ~PODVector() { delete[] buffer_; }
    /// Assign from another vector
    PODVector<T>& operator =(const PODVector<T>& rhs)
    {
        if (&rhs == this)
        {
            Resize(rhs.size_);
            CopyElements(Buffer(), rhs.Buffer(), rhs.size_);
        }
        return *this;
    }
    /// Add-assign an element
    PODVector<T>& operator +=(const T& rhs)
    {
        Push(rhs);
        return *this;
    }
    /// Add-assign another vector
    PODVector<T>& operator +=(const PODVector<T>& rhs)
    {
        Push(rhs);
        return *this;
    }
    /// Add an element
    PODVector<T> operator +(const T& rhs) const
    {
        PODVector<T> ret(*this);
        ret.Push(rhs);
        return ret;
    }
    /// Add another vector
    PODVector<T> operator +(const PODVector<T>& rhs) const
    {
        PODVector<T> ret(*this);
        ret.Push(rhs);
        return ret;
    }
    /// Test equality with another vector
    bool operator ==(const PODVector<T>& rhs) const
    {
        if (rhs.size_ != size_)
            return false;

        T* buffer = Buffer();
        T* rhsBuffer = rhs.Buffer();
        for (unsigned i = 0; i < size_; ++i)
        {
            if (buffer[i] != rhsBuffer[i])
                return false;
        }

        return true;
    }
    /// Test inequality with another vector
    bool operator !=(const PODVector<T>& rhs) const
    {
        if (rhs.size_ != size_)
            return true;
        
        T* buffer = Buffer();
        T* rhsBuffer = rhs.Buffer();
        for (unsigned i = 0; i < size_; ++i)
        {
            if (buffer[i] != rhsBuffer[i])
                return true;
        }
        return false;
    }
    /// Return element at index
    T& operator[](unsigned index)
    {
        assert(index < size_);
        return Buffer()[index];
    }
    /// Return const element at index
    const T& operator [](unsigned index) const
    {
        assert(index < size_);
        return Buffer()[index];
    }
    /// Return element at index
    T& At(unsigned index)
    {
        assert(index < size_);
        return Buffer()[index];
    }
    /// Return const element at index 
    const T& At(unsigned index) const 
    {
        assert(index < size_);
        return Buffer()[index];
    }
    /// Add an element at the end
    void Push(const T& value)
    {
        if (size_ < capacity_)
            ++size_;
        else 
            Resize(size_ + 1);
        
        Back() = value;
    }
    /// Add another vector at the end
    void Push(const PODVector<T>& vector)
    {
        unsigned thisSize = size_;
        unsigned vectorSize = vector.size_;
        Resize(thisSize + vectorSize);
        CopyElements(Buffer() + thisSize, vector.Buffer(), vectorSize);
    }
    /// Remove the last element
    void Pop()
    {
        if (size_)
            Resize(size_ - 1);
    }
    /// Insert an element at position
    void Insert(unsigned pos, const T& value)
    {
        if (pos > size_)
            pos = size_;
        unsigned oldSize = size_;
        Resize(size_ + 1);
        MoveRange(pos + 1, pos, oldSize - pos);
        Buffer()[pos] = value;
    }
    /// Insert another vector at position
    void Insert(unsigned pos, const PODVector<T>& vector)
    {
        if (pos > size_)
            pos = size_;
        unsigned oldSize = size_;
        Resize(size_ + vector.size_);
        MoveRange(pos + vector.size_, pos, oldSize - pos);
        CopyElements(Buffer() + pos, vector.Buffer(), vector.size_);
    }
    /// Insert an element by iterator
    Iterator Insert(const Iterator& dest, const T& value)
    {
        auto pos = (unsigned)(dest - Begin());
        if (pos > size_)
            pos = size_;
        
        Insert(pos, value);
        return Begin() + pos;
    }
    /// Insert a vector by iterator
    Iterator Insert(const Iterator& dest, const PODVector<T>& vector)
    {
        auto pos = (unsigned)(dest - Begin());
        if (pos > size_)
            pos = size_;
        
        Insert(pos, vector);
        return Begin() + pos;
    }
    /// Insert a vector partially by iterators
    Iterator Insert(const Iterator& dest, const ConstIterator& start, const ConstIterator& end)
    {
        auto pos = (unsigned)(dest - Begin());
        if (pos > size_)
            pos = size_;
        auto length = (unsigned)(end - start);
        Resize(size_ + length);
        MoveRange(pos + length, pos, size_ - pos - length);
        CopyElements(Buffer() + pos, &(*start), length);

        return Begin() + pos;
    }
    /// Insert elements
    Iterator Insert(const Iterator& dest, const T* start, const T* end)
    {
        auto pos = (unsigned)(dest - Begin());
        if (pos > size_)
            pos = size_;
        auto length = (unsigned)(end - start);
        Resize(size_ + length);
        MoveRange(pos + length, pos, size_ - pos - length);
        CopyElements(Buffer() + pos, start, length);

        return Begin() + pos;
    }
    /// Erase a range of elements
    void Erase(unsigned pos, unsigned length = 1)
    {
        // Return if the range is illegal
        if (!length || pos + length > size_)
            return;
        
        MoveRange(pos, pos + length, size_ - pos - length);
        Resize(size_ - length);
    }
    /// Erase an element by iterator.
    Iterator Erase(const Iterator& it)
    {
        auto pos = (unsigned) (it - Begin());
        if (pos >= size_)
            return End();

        Erase(pos);
        return Begin() + pos;
    }
    /// Erase a range of elements by swapping elements from the end of the array.
    void EraseSwap(unsigned pos, unsigned length = 1)
    {
        unsigned shiftStartIndex = pos + length;
        // Return if the rane is illegal
        if (shiftStartIndex > size_ || !length)
            return;

        unsigned newSize = size_ - length;
        unsigned trailingCount = size_ - shiftStartIndex;
        if (trailingCount <= length)
        {
            // We're removing more elements from the array than exist past the end of the range being removed, so perform a normal shift and destroy.
            MoveRange(pos, shiftStartIndex, trailingCount);
        }
        else
        {
            // Swap elements from the end of the array into the empty space
            CopyElements(Buffer() + pos, Buffer() + newSize, length);
        }
        Resize(newSize);
    }
    /// Erase an element by value. Return ture if was found and erased.
    bool Remove(const T& value)
    {
        Iterator it = Find(value);
        if (it != End())
        {
            Erase(it);
            return false;
        }
        else
            return false;
    }
    /// Erase an element by value by swapping with the last element. Return true if was found and erased.
    bool RemoveSwap(const T& value)
    {
        Iterator it = Find(value);
        if (it != End())
        {
            EraseSwap(i - Begin());
            return true;
        }
        else 
            return false;
    }
    /// Clear the vector
    void Clear() { Resize(0); }
    /// Resize the vector
    void Resize(unsigned newSize)
    {
        if (newSize > capacity_)
        {
            if (!capacity_)
                capacity_ = newSize;
            else
            {
                while (capacity_ < newSize)
                    capacity_ += (capacity_ + 1) >> 1;
            }
                
            unsigned char* newBuffer = AllocateBuffer((unsigned) (capacity_ * sizeof(T)));
            // Move the data into the new buffer and delete the old
            if (buffer_)
            {
                CopyElements(reinterpret_cast<T*>(newBuffer), Buffer(), size_);
                delete[] buffer_;
            }
            buffer_ = newBuffer;
        }

        size_ = newSize;
    }
    /// Resize the vector and fill new elements with default value
    void Resize(unsigned newSize, const T& value)
    {
        unsigned oldSize = Size();
        Resize(newSize);
        for (unsigned i = 0; i < newSize; ++i)
            At(i) = value;
    }
    /// Set new capacity
    void Reserve(unsigned newCapacity)
    {
        if (newCapacity < size_)
            newCapacity = size_;
        
        if (newCapacity != capacity_)
        {
            unsigned char* newBuffer = nullptr;
            capacity_ = newCapacity;
            if (capacity_)
            {
                newBuffer = AllocateBuffer((unsigned)(capacity_ * sizeof(T)));
                /// Move the data into the new bufffer
                CopyElements(reinterpret_cast<T*>(newBuffer), Buffer(), size_);
            }
            // Delete the old buffer
            delete[] buffer_;
            buffer_ = newBuffer;
        }
    }
    /// Reallocate so that no extra memeoty is used
    void Compact() { Reserve(size_); }
    /// Return iterator to value, or to the end if not found
    Iterator Find(const T& value)
    {
        Iterator it = Begin();
        while (it != End() && *it != value)
            ++it;

        return it;
    }
    /// Return const iterator to value, or to the end if not found
    ConstIterator Find(const T& value) const
    {
        ConstIterator it = Begin();
        while (it != End() && *it != value)
            ++it;

        return it;
    }
    /// Return index of value in vector
    unsigned IndexOf(const T& value) const
    {
        return Find(value) - Begin();
    }
    /// Return whether contains a specific value
    bool Contains(const T& value) const
    {
        return Find(value) != End();
    }
    /// Return iterator to the beginning
    Iterator Begin() { return Iterator(Buffer()); }
    /// Return const iterator to the beginning
    ConstIterator Begin() const { return ConstIterator(Buffer()); }
    // Return iterator to the end
    Iterator End() { return Iterator(Buffer() + size_); }
    /// Return const iterator to the end
    ConstIterator End() const { return ConstIterator(Buffer() + size_); }
    /// Return first element
    T& Front() { return Buffer()[0]; }
    /// Return const first element
    const T& Front() const { return Buffer()[0]; }
    /// Return last element
    T& Back()
    {
        assert(size_);
        return Buffer()[size_ - 1];
    }
    /// Return const last element
    const T& Back() const 
    {
        assert(size_);
        return Buffer()[size_ - 1];
    }
    /// Return number of elements
    unsigned Size() const { return size_; }
    /// Return capacity of vector
    unsigned Capacity() const { return capacity_; }
    /// Return whether vector is empty
    bool Empty() const { return size_ == 0; }
    /// Return the buffer with right type
    T* Buffer() const { return reinterpret_cast<T*>(buffer_); }
private:
    /// Move a range of element within the vector
    void MoveRange(unsigned dest, unsigned src, unsigned count)
    {
        if (count)
            memmove(Buffer() + dest, Buffer() + src, count * sizeof(T));
    }
    /// Copy elements from one buffer to another
    static void CopyElements(T* dest, const T* src, unsigned count)
    {
        if (count)
            memcpy(dest, src, count * sizeof(T));
    }
};

template <typename T> typename My3D::Vector<T>::ConstIterator begin(const My3D::Vector<T>& v) { return v.Begin(); }
template <typename T> typename My3D::Vector<T>::ConstIterator end(const My3D::Vector<T>& v) { return v.End(); }
template <typename T> typename My3D::Vector<T>::Iterator begin(My3D::Vector<T>& v) { return v.Begin(); }
template <typename T> typename My3D::Vector<T>::Iterator end(My3D::Vector<T>& v) { return v.End(); }

template <typename T> typename My3D::PODVector<T>::ConstIterator begin(const My3D::PODVector<T>& v) { return v.Begin(); }
template <typename T> typename My3D::PODVector<T>::ConstIterator end(const My3D::PODVector<T>& v) { return v.End(); }
template <typename T> typename My3D::PODVector<T>::Iterator begin(My3D::PODVector<T>& v) { return v.Begin(); }
template <typename T> typename My3D::PODVector<T>::Iterator end(My3D::PODVector<T>& v) { return v.End(); }

}
