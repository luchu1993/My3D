//
// Created by luchu on 2022/1/5.
//

#pragma once

#include "Math/MathDefs.h"
#include "Math/Vector2.h"


namespace My3D
{
    class MY3D_API Vector3
    {
    public:
        /// Construct a zero vector
        Vector3() noexcept : x_(0.0f), y_(0.0f), z_(0.0f) {}
        /// Construct from another vector
        Vector3(const Vector3& vector3) = default;
        /// Construct from a two-dimensional vector and Z coordinate.
        Vector3(const Vector2& vector, float z) noexcept
            : x_(vector.x_), y_(vector.y_), z_(z) { }
        /// Construct from a two-dimensional vector
        Vector3(const Vector2& vector) noexcept
             : x_(vector.x_), y_(vector.y_), z_(0.0f) { }
        /// Construct from coordinates.
        Vector3(float x, float y, float z) noexcept : x_(x), y_(y),z_(z) { }
        /// Construct from two-dimensional coordinates (for Urho2D).
        Vector3(float x, float y) noexcept : x_(x), y_(y), z_(0.0f) { }
        /// Construct from a float array.
        explicit Vector3(const float* data) noexcept : x_(data[0]), y_(data[1]), z_(data[2]) { }
        /// Multiply with a scalar.
        Vector3 operator *(float rhs) const { return Vector3(x_ * rhs, y_ * rhs, z_ * rhs); }
        /// Multiply with a vector.
        Vector3 operator *(const Vector3& rhs) const { return Vector3(x_ * rhs.x_, y_ * rhs.y_, z_ * rhs.z_); }
        /// Return negation.
        Vector3 operator -() const { return Vector3(-x_, -y_, -z_); }
        /// Subtract a vector.
        Vector3 operator -(const Vector3& rhs) const { return Vector3(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_); }
        /// Calculate cross product.
        Vector3 CrossProduct(const Vector3& rhs) const
        {
            return Vector3(
                    y_ * rhs.z_ - z_ * rhs.y_,
                    z_ * rhs.x_ - x_ * rhs.z_,
                    x_ * rhs.y_ - y_ * rhs.x_
            );
        }
        /// Return absolute vector.
        Vector3 Abs() const { return Vector3(My3D::Abs(x_), My3D::Abs(y_), My3D::Abs(z_)); }
        /// Return whether any component is NaN.
        bool IsNaN() const { return My3D::IsNaN(x_) || My3D::IsNaN(y_) || My3D::IsNaN(z_); }
        /// Return whether any component is Inf.
        bool IsInf() const { return My3D::IsInf(x_) || My3D::IsInf(y_) || My3D::IsInf(z_); }
        /// Return normalized to unit length.
        Vector3 Normalized() const
        {
            const float lenSquared = LengthSquared();
            if (!My3D::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }
            else
                return *this;
        }
        /// Test equality with another vector without epsilon
        bool operator ==(const Vector3& rhs) const
        {
            return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_;
        }
        /// Test inequality with another vector without epsilon
        bool operator !=(const Vector3& rhs) const
        {
            return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_;
        }
        /// Return length.
        float Length() const { return sqrtf(x_ * x_ + y_ * y_ + z_ * z_); }
        /// Return squared length.
        float LengthSquared() const { return x_ * x_ + y_ * y_ + z_ * z_; }
        /// Calculate dot product.
        float DotProduct(const Vector3& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_; }
        /// Return float data.
        const float* Data() const { return &x_; }
        /// Return as string.
        String ToString() const;
        /// Return hash value for HashSet & HashMap.
        unsigned ToHash() const
        {
            unsigned hash = 37;
            hash = 37 * hash + FloatToRawIntBits(x_);
            hash = 37 * hash + FloatToRawIntBits(y_);
            hash = 37 * hash + FloatToRawIntBits(z_);

            return hash;
        }

        /// X coordinate.
        float x_;
        /// Y coordinate.
        float y_;
        /// Z coordinate.
        float z_;

        /// Zero vector.
        static const Vector3 ZERO;
        /// (-1,0,0) vector.
        static const Vector3 LEFT;
        /// (1,0,0) vector.
        static const Vector3 RIGHT;
        /// (0,1,0) vector.
        static const Vector3 UP;
        /// (0,-1,0) vector.
        static const Vector3 DOWN;
        /// (0,0,1) vector.
        static const Vector3 FORWARD;
        /// (0,0,-1) vector.
        static const Vector3 BACK;
        /// (1,1,1) vector.
        static const Vector3 ONE;
    };

    class MY3D_API IntVector3
    {
    public:
        /// Construct a zero vector.
        IntVector3() noexcept
            : x_(0)
            , y_(0)
            , z_(0)
        {
        }
        /// Construct from coordinates.
        IntVector3(int x, int y, int z) noexcept
            : x_(x)
            , y_(y)
            , z_(z)
        {
        }
        /// Construct from an int array.
        explicit IntVector3(const int* data) noexcept
            : x_(data[0])
            , y_(data[1])
            , z_(data[2])
        {
        }
        /// Copy-construct from another vector.
        IntVector3(const IntVector3& rhs) noexcept = default;

        /// Assign from another vector.
        IntVector3& operator =(const IntVector3& rhs) noexcept = default;
        /// Return integer data.
        const int* Data() const { return &x_; }
        /// Return as string.
        String ToString() const;
        /// Return hash value for HashSet & HashMap.
        unsigned ToHash() const { return (unsigned)x_ * 31 * 31 + (unsigned)y_ * 31 + (unsigned)z_; }
        /// Return length.
        float Length() const { return sqrtf((float)(x_ * x_ + y_ * y_ + z_ * z_)); }
        /// Test for equality with another vector.
        bool operator ==(const IntVector3& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_; }
        /// Test for inequality with another vector.
        bool operator !=(const IntVector3& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_; }

        /// X coordinate.
        int x_;
        /// Y coordinate.
        int y_;
        /// Z coordinate.
        int z_;

        /// Zero vector.
        static const IntVector3 ZERO;
        /// (-1,0,0) vector.
        static const IntVector3 LEFT;
        /// (1,0,0) vector.
        static const IntVector3 RIGHT;
        /// (0,1,0) vector.
        static const IntVector3 UP;
        /// (0,-1,0) vector.
        static const IntVector3 DOWN;
        /// (0,0,1) vector.
        static const IntVector3 FORWARD;
        /// (0,0,-1) vector.
        static const IntVector3 BACK;
        /// (1,1,1) vector.
        static const IntVector3 ONE;
    };
}
