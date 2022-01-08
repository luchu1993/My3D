//
// Created by luchu on 2022/1/5.
//

#pragma once

#include "My3D.h"
#include "Math/MathDefs.h"


namespace My3D
{
    class String;

    class MY3D_API IntVector2
    {
    public:
        /// X coordinate
        int x_;
        /// Y coordinate
        int y_;
    };

    /// Two-dimensional vector
    class MY3D_API Vector2
    {
    public:
        /// Construct a zero vector
        Vector2() noexcept : x_(0.0f) , y_(0.0f) { }
        /// Copy construct from another vector
        Vector2(const Vector2& vector) noexcept = default;
        /// Construct from an IntVector2
        explicit Vector2(const IntVector2& vector) noexcept : x_(float(vector.x_)), y_(float(vector.y_)) { }
        /// Construct from coordinates
        Vector2(float x, float y) noexcept : x_(x), y_(y) { }
        /// Construct from a float array
        explicit Vector2(const float * data) noexcept : x_(data[0]), y_(data[1]) { }
        /// Assign from another vector
        Vector2& operator =(const Vector2& rhs) noexcept = default;
        /// Test for equality with another vector without epsilon
        bool operator ==(const Vector2& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }
        /// Test for inequality with another vector without epsilon
        bool operator !=(const Vector2& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }
        /// Add a vector
        Vector2 operator +(const Vector2& rhs) const { return Vector2(x_ + rhs.x_, y_ + rhs.y_); }
        /// Return negation
        Vector2 operator -() const { return Vector2(-x_, -y_); }
        /// Subtract a vector
        Vector2 operator -(const Vector2& rhs) const { return Vector2(x_ - rhs.x_, y_ - rhs.y_); }
        /// Multiply with a scalar
        Vector2 operator *(float rhs) const { return Vector2(rhs * x_, rhs * y_); }
        /// Multiply with a vector
        Vector2 operator *(const Vector2& rhs) { return Vector2(x_ * rhs.x_, y_ * rhs.y_); }
        /// Divide by a scalar
        Vector2 operator /(float rhs) const { return Vector2(x_ / rhs, y_ / rhs); }
        /// Divide by a vector
        Vector2 operator /(const Vector2& rhs) { return Vector2(x_ / rhs.x_, y_ / rhs.y_); }
        /// Add-assign a vector
        Vector2& operator +=(const Vector2& rhs)
        {
            x_ += rhs.x_;
            y_ += rhs.y_;
            return *this;
        }
        /// Subtract-assign a vector
        Vector2& operator-=(const Vector2& rhs)
        {
            x_ -= rhs.x_;
            y_ -= rhs.y_;
            return *this;
        }
        /// Multiply-assign a scalar
        Vector2& operator *=(float rhs)
        {
            x_ *= rhs;
            y_ *= rhs;
            return *this;
        }
        /// Multiply-assign a vector
        Vector2& operator *=(const Vector2& rhs)
        {
            x_ *= rhs.x_;
            y_ *= rhs.y_;
            return *this;
        }
        /// Divide-assign a scalar
        Vector2& operator /=(float rhs)
        {
            float inv = 1.0f / rhs;
            x_ *= inv;
            y_ *= inv;
            return *this;
        }
        /// Divide-assign a vector
        Vector2& operator /=(const Vector2& rhs)
        {
            x_ /= rhs.x_;
            y_ /= rhs.y_;
            return *this;
        }
        /// Normalize to unit length
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!My3D::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                x_ *= invLen;
                y_ *= invLen;
            }
        }
        /// Return length
        float Length() const { return sqrtf(x_ * x_ + y_ * y_); }
        /// Return squared length
        float LengthSquared() const { return x_ * x_ + y_ * y_; }
        /// Calculate dor product
        float DotProduct(const Vector2& rhs) const { return x_ * rhs.x_ + y_ * rhs.y_; }
        /// Calculate absolute dot product
        float AbsDotProduct(const Vector2& rhs) const { return My3D::Abs(x_ * rhs.x_) + My3D::Abs(y_ * rhs.y_); }
        /// Return absolute vector
        Vector2 Abs() const { return Vector2(My3D::Abs(x_), My3D::Abs(y_)); }
        /// Linear interpolation with another vector
        Vector2 Lerp(const Vector2& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }
        /// Return the angle between this vector and another vector in degrees
        float Angle(const Vector2& rhs) const { return My3D::Acos(DotProduct(rhs) / Length() * rhs.Length()); }
        /// Test for equality with another vector with epsilon
        bool Equals(const Vector2& rhs) const { return My3D::Equals(x_, rhs.x_) && My3D::Equals(y_, rhs.y_); }
        /// Return whether any component is NaN
        bool IsNaN() const { return My3D::IsNaN(x_) || My3D::IsNaN(y_); }
        /// Return whether any component is Inf
        bool IsInf() const { return My3D::IsInf(x_) || My3D::IsInf(y_); }
        /// Return normalized to unit length
        Vector2 Normalzied() const
        {
            const float  lenSquared = LengthSquared();
            if (!My3D::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                const float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }
            return *this;
        }
        /// Return float data
        const float * Data() const { return &x_; }
        /// Return as string
        String ToString() const;
        /// X coordinate
        float x_;
        /// Y coordinate
        float y_;
        /// Zero vector
        static const Vector2 ZERO;
        //// Left vector
        static const Vector2 LEFT;
        /// Right vector
        static const Vector2 RIGHT;
        /// Up vector
        static const Vector2 UP;
        /// Down vector
        static const Vector2 DOWN;
        /// One vector
        static const Vector2 ONE;
    };

    /// Multiply Vector2 with a scalar.
    inline Vector2 operator *(float lhs, const Vector2& rhs) { return rhs * lhs; }
}
