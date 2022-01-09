//
// Created by luchu on 2022/1/5.
//

#pragma once

#include "Vector3.h"

namespace My3D
{

class MY3D_API Vector4
{
public:
    /// Construct a zero vector.
    Vector4() noexcept
        : x_(0.0f)
        , y_(0.0f)
        , z_(0.0f)
        , w_(0.0f)
    {
    }
    /// Copy-construct from another vector.
    Vector4(const Vector4& vector) noexcept = default;
    /// Construct from a 3-dimensional vector and the W coordinate.
    Vector4(const Vector3& vector, float w) noexcept
        : x_(vector.x_)
        , y_(vector.y_)
        , z_(vector.z_)
        , w_(w)
    {
    }
    /// Construct from coordinates.
    Vector4(float x, float y, float z, float w) noexcept
        : x_(x)
        , y_(y)
        , z_(z)
        , w_(w)
    {
    }
    /// Construct from a float array.
    explicit Vector4(const float* data) noexcept
        : x_(data[0])
        , y_(data[1])
        , z_(data[2])
        , w_(data[3])
    {
    }
    /// Test equality with another vector without epsilon
    bool operator ==(const Vector4& rhs) const
    {
        return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_ && w_ == rhs.w_;
    }
    /// Test inequality with another vector without epsilon
    bool operator !=(const Vector4& rhs) const
    {
        return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_ || w_ != rhs.w_;
    }

    /// Return float data.
    const float* Data() const { return &x_; }
    /// Return as string.
    String ToString() const;

    /// X coordinate.
    float x_;
    /// Y coordinate.
    float y_;
    /// Z coordinate.
    float z_;
    /// W coordinate
    float w_;

    /// Zero vector.
    static const Vector4 ZERO;
    /// (1,1,1) vector.
    static const Vector4 ONE;
};

}
