//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Container/Vector.h"

#include <cstring>
#include <cctype>
#include <cstdarg>
#include <cassert>


namespace My3D
{

static const int CONVERSION_BUFFER_LENGTH = 128;
static const int MATRIX_CONVERSION_BUFFER_LENGTH = 256;

class StringHash;
class String;

template <typename T, typename U> class HashMap;
/// Map of string
using StringMap = HashMap<StringHash, String>;

/// String class
class MY3D_API String
{
public:
    using Iterator = RandomAccessIterator<char>;
    using ConstIterator = RandomAccessConstIterator<char>;

    /// Construct empty
    String() noexcept : length_(0), capacity_(0), buffer_(&endZero) { }
    /// Construct from another string
    String(const String& str) : length_(0), capacity_(0), buffer_(&endZero)
    {
        *this = str;
    }
    /// Move-construct from another string
    String(String&& str) noexcept : length_(0), capacity_(0), buffer_(&endZero)
    {
        Swap(str);
    }
    /// Construct from a C string.
    String(const char* str) : length_(0), capacity_(0), buffer_(&endZero)
    {
        *this = (const char*) str;
    }
    /// Construct from a C string.
    String(char* str) : length_(0), capacity_(0), buffer_(&endZero)
    {
        *this = (const char*) str;
    }
    /// Construct from a char array and length
    String(const char* str, unsigned length) : length_(0), capacity_(0), buffer_(&endZero) { }

    /// Construct from a null-terminated wide character array
    explicit String(const wchar_t* str)
        : length_(0)
        , capacity_(0)
        , buffer_(&endZero)
    {

    }
    /// Construct from an integer
    explicit String(int value);
    /// Construct from a short integer
    explicit String(short value);
    /// Construct from a long integer
    explicit String(long value);
    /// Construct from a long long integer
    explicit String(long long value);
    /// Construct from an unsigned integer
    explicit String(unsigned value);
    /// Construct from an unsigned short integer.
    explicit String(unsigned short value);
    /// Construct from an unsigned long integer.
    explicit String(unsigned long value);
    /// Construct from an unsigned long long integer.
    explicit String(unsigned long long value);
    /// Construct from a float.
    explicit String(float value);
    /// Construct from a double.
    explicit String(double value);
    /// Construct from a bool.
    explicit String(bool value);
    /// Construct from a character.
    explicit String(char value);
    /// Construct from a character and fill length.
    explicit String(char value, unsigned length);
    /// Construct from a convertible value
    template<typename T> explicit String(const T& value)
        : length_(0)
        , capacity_(0)
        , buffer_(&endZero)
    {
        *this = value.ToString();
    }
    /// Destruct
    ~String()
    {
        if (capacity_) delete[] buffer_;
    }
    /// Assign a string
    String& operator=(const String& rhs)
    {
        if (&rhs != this)
        {
            Resize(rhs.length_);
            CopyChars(buffer_, rhs.buffer_, rhs.length_);
        }

        return *this;
    }
    /// Move-assign a string
    String& operator=(String&& rhs) noexcept
    {
        Swap(rhs);
        return *this;
    }
    /// Construct from a C string
    String& operator=(const char* rhs)
    {
        unsigned rhsLength = CStringLength(rhs);
        Resize(rhsLength);
        CopyChars(buffer_, rhs, rhsLength);

        return *this;
    }
    /// Add-assign a string
    String& operator +=(const String& rhs)
    {
        unsigned oldLength = length_;
        Resize(length_ + rhs.length_);
        CopyChars(buffer_ + oldLength, rhs.buffer_, rhs.length_);

        return *this;
    }
    /// Add-assign a C string
    String& operator +=(const char* rhs)
    {
        unsigned rhsLength = CStringLength(rhs);
        unsigned oldLength = length_;
        Resize(length_ + rhsLength);
        CopyChars(buffer_ + oldLength, rhs, rhsLength);

        return *this;
    }
    /// Add-assign a character
    String& operator +=(char rhs)
    {
        unsigned oldLength = length_;
        Resize(length_ + 1);
        buffer_[oldLength] = rhs;

        return *this;
    }
    /// Add s string
    String operator +(const String& rhs) const
    {
        String ret;
        ret.Resize(length_ + rhs.length_);
        CopyChars(ret.buffer_, buffer_, length_);
        CopyChars(ret.buffer_ + length_, rhs.buffer_, rhs.length_);

        return ret;
    }
    /// Add a C string
    String operator +(const char* rhs) const
    {
        unsigned rhsLength = CStringLength(rhs);
        String ret;
        ret.Resize(length_ + rhsLength);
        CopyChars(ret.buffer_, buffer_, length_);
        CopyChars(ret.buffer_ + length_, rhs, rhsLength);

        return ret;
    }
    /// Convert to a C string
    const char* operator*() const { return CString(); }
    /// Test equality with another string
    bool operator ==(const String& rhs) const { return strcmp(CString(), rhs.CString()) == 0; }
    /// Test equality with C string
    bool operator ==(const char* rhs) const { return strcmp(CString(), rhs) == 0; }
    /// Test inequality with another string
    bool operator !=(const String& rhs) const { return strcmp(CString(), rhs.CString()) != 0; }
    /// Test inequality with C string
    bool operator !=(const char* rhs) const { return strcmp(CString(), rhs) != 0; }
    /// Return char at index
    char& operator [](unsigned index)
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return const char at index
    const char& operator [](unsigned index) const
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return char at index
    char& At(unsigned index)
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return const char at index
    const char& At(unsigned index) const
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Append a string
    String& Append(const String& str);
    /// Append a C string
    String& Append(const char* str);
    /// Append characters
    String& Append(const char* str, unsigned length);
    /// Append a character
    String& Append(char c);
    /// Return the C string.
    const char* CString() const { return buffer_; }
    /// Return length
    unsigned Length() const { return length_; }
    /// Return buffer capacity
    unsigned Capacity() const { return capacity_; }
    /// Return whether the string is empty
    bool Empty() const { return length_ == 0; }
    /// Return comparison result with a string.
    int Compare(const String& str, bool caseSensitive = true) const;
    /// Return comparison result with a C string.
    int Compare(const char* str, bool caseSensitive = true) const;
    /// Swap with another string
    void Swap(String& str);
    /// Set new capacity.
    void Reserve(unsigned newCapacity);
    /// CLear the sting
    void Clear();
    /// Reallocate so that no extra memory is used.
    void Compact();
    /// Resize the string.
    void Resize(unsigned newLength);
    /// Return iterator to the beginning
    Iterator Begin() { return Iterator(buffer_); }
    /// Return const iterator to the beginning
    ConstIterator Begin() const { return ConstIterator(buffer_); }
    /// Return iterator to the end
    Iterator End() { return Iterator(buffer_ + length_); }
    /// Return const iterator to the end
    ConstIterator End() const { return ConstIterator(buffer_ + length_); }
    /// Return first char, or 0 if empty
    char Front() const { return buffer_[0]; }
    /// Return last char, or 0 if empty
    char Back() const { return length_ ? buffer_[length_ - 1] : buffer_[0]; }
    /// Return a substring from position to end
    String Substring(unsigned pos) const;
    /// Return a substring with length from position
    String Substring(unsigned pos, unsigned length) const;
    /// Return string in uppercase
    String ToUpper() const;
    /// Return string in lowercase
    String ToLower() const;
    /// Return string with whitespace trimmed from the beginning and the end.
    String Trimmed() const;
    /// Return hash value for HashSet & HashMap
    unsigned ToHash() const
    {
        unsigned hash = 0;
        const char* ptr = buffer_;
        while (*ptr)
        {
            hash = *ptr + (hash << 6u) + (hash << 16u) - hash;
            ++ptr;
        }

        return hash;
    }
    /// Return length of C string
    static unsigned CStringLength(const char* str) { return str ? (unsigned) strlen(str) : 0; }
    /// Append to string using formatting
    String& AppendWithFormat(const char* formatString, ...);
    /// Append to string using variable arguments
    String& AppendWithFormatArgs(const char* formatString, va_list args);
    /// Compare two C String
    static int Compare(const char* lhs, const char* rhs, bool casSensitive);
    /// Position for NOT FOUND
    static const unsigned NPOS = 0xffffffff;
    /// Initial dynamic allocation size
    static const unsigned MIN_CAPACITY = 8;
    /// Empty string
    static const String EMPTY;
private:
    /// Move a range of characters within the string.
    void MoveRange(unsigned  dest, unsigned src, unsigned count)
    {
        if (count)
            memmove(buffer_ + dest, buffer_ + src, count);
    }
    /// Copy chars from one buffer to another.
    static void CopyChars(char* dest, const char* src, unsigned count)
    {
        if (count)
            memcpy(dest, src, count);
    }
    /// String length.
    unsigned length_;
    /// Capacity, zero if buffer not allocated.
    unsigned capacity_;
    /// String buffer, point to &endZero if buffer is not allocated.
    char* buffer_;
    /// End zero for empty strings.
    static char endZero;
};

/// Add a string to a C string
inline String operator +(const char* lhs, const String& rhs)
{
    String ret(lhs);
    ret += rhs;
    return ret;
}

/// Wide character string. Only meant for converting from String and passing to the operating system where necessary.
class MY3D_API WString
{
public:
    /// Construct empty
    WString();
    /// Construct from a string
    explicit WString(const String& str);
    /// Destruct
    ~WString() noexcept;
    /// Return char at index
    wchar_t& operator[](unsigned index)
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return const char at index
    const wchar_t& operator[](unsigned index) const
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return char at index
    wchar_t& At(unsigned index)
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return const char at index
    const wchar_t& At(unsigned index) const
    {
        assert(index < length_);
        return buffer_[index];
    }
    /// Return c string
    const wchar_t* operator*() const { return buffer_; }
    /// Return length
    unsigned Length() const { return length_; }
    /// Return whether the string is empty
    bool Empty() const { return length_ == 0; }
    /// Return character data
    const wchar_t* CString() const { return buffer_; }
private:
    /// String length
    unsigned length_;
    /// String buffer, null if not allocated
    wchar_t* buffer_;
};

}

