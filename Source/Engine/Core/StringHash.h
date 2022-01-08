//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "My3D.h"
#include "Container/String.h"

namespace My3D
{
class StringHashRegister;

/// 32-bit hash value for a string.
class MY3D_API StringHash
{
public:
    /// Construct with zero value
    StringHash() noexcept : value_(0) { }
    /// Construct from another hash
    StringHash(const StringHash& rhs) noexcept = default;
    /// Construct with an initial value
    explicit StringHash(unsigned value) noexcept : value_(value) { }
    /// Construct from a C string
    StringHash(const char* str) noexcept;
    /// Construct from a string
    StringHash(const String& str) noexcept;
    /// Assign from another hash
    StringHash& operator =(const StringHash& rhs) noexcept = default;
    /// Add a hash
    StringHash operator +(const StringHash& rhs) const
    {
        StringHash ret;
        ret.value_ = value_ + rhs.value_;
        return ret;
    }
    /// Add-assign a hash
    StringHash& operator +=(const StringHash& rhs)
    {
        value_ += rhs.value_;
        return *this;
    }
    /// Test equality with another hash
    bool operator ==(const StringHash& rhs) const { return value_ == rhs.value_; }
    /// Test inequality with another hash
    bool operator !=(const StringHash& rhs) const { return value_ != rhs.value_; }
    /// Test less than another hash
    bool operator <(const StringHash& rhs) const { return value_ < rhs.value_; }
    /// Test greater than another hash
    bool operator >(const StringHash& rhs) const { return value_ > rhs.value_; }
    /// Return true if nonzero hash value
    explicit operator bool() const { return value_ != 0; }
    /// return as string
    String ToString() const;
    /// Return hash value for Hashset & Hashmap
    unsigned ToHash() const { return value_; }
    /// Return hash value
    unsigned Value() const { return value_; }
    /// Calculate hash value from a C string
    static unsigned Calculate(const char* str, unsigned hash = 0);
    /// Get global StringHashRegister. Used for debug only.
    static StringHashRegister* GetGlobalStringHashRegister() { return nullptr; }
    /// Zero hash
    static const StringHash ZERO;
private:
    unsigned value_;
};

}

