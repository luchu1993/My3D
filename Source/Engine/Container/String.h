//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"

#include <cstring>
#include <cctype>
#include <cstdarg>

namespace My3D
{

static const int CONVERSION_BUFFER_LENGTH = 128;
static const int MATRIX_CONVERSION_BUFFER_LENGTH = 256;

class StringHash;

class MY3D_API String
{
public:
    String() noexcept : length_(0), capacity_(0), buffer_(&endZero) { }

    String(const String& str) : length_(0), capacity_(0), buffer_(&endZero)
    {
        *this = str;
    }

    String(String&& str) noexcept : length_(0), capacity_(0), buffer_(&endZero)
    {
        Swap(str);
    }

    String(const char* str) : length_(0), capacity_(0), buffer_(&endZero)
    {
        *this = (const char*) str;
    }

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
    void Swap(String& str);
    /// CLear the sting
    void Clear();
    void Compact();
    void Resize(unsigned newLength);
    char Front() const { return buffer_[0]; }
    char Back() const { return length_ ? buffer_[length_ - 1] : buffer_[0]; }
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

    static void CopyChars(char* dest, const char* src, unsigned count)
    {
        char* end = dest + count;
        while (dest != end)
        {
            *dest = *src;
            ++dest;
            ++src;
        }
    }

    unsigned length_;
    unsigned capacity_;
    char* buffer_;

    static char endZero;
};


/// Add a string to a C string
inline String operator +(const char* lhs, const String& rhs)
{
    String ret(lhs);
    ret += rhs;
    return ret;
}
}

