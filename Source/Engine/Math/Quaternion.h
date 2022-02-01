//
// Created by luchu on 2022/1/15.
//

#pragma once

#include "Math/Matrix3.h"


namespace My3D
{
    /// Rotation represented as a four-dimensional normalized vector.
    class MY3D_API Quaternion
    {
    public:
        /// Construct an identify quaternion
        Quaternion() noexcept
            : w_(1.0f)
            , x_(0.0f)
            , y_(0.0f)
            , z_(0.0f)
        {
        }
        /// Copy-construct from another quaternion
        Quaternion(const Quaternion& quat) noexcept
            : w_(quat.w_)
            , x_(quat.x_)
            , y_(quat.y_)
            , z_(quat.z_)
        {
        }
        /// Construct from values
        Quaternion(float w, float x, float y, float z)
            : w_(w)
            , x_(x)
            , y_(y)
            , z_(z)
        {
        }
        /// Construct from float array
        explicit Quaternion(const float* data) noexcept
            : w_(data[0])
            , x_(data[1])
            , y_(data[2])
            , z_(data[3])
        {
        }
        /// Construct from an angle (in degrees) and axis
        Quaternion(float angle, const Vector3& axis) noexcept
        {
            FromAngleAxis(angle, axis);
        }
        /// Construct from an angle (in degrees, for Urho2D).
        explicit Quaternion(float angle) noexcept
        {
            FromAngleAxis(angle, Vector3::FORWARD);
        }
        /// Construct from Euler angles (in degrees). Equivalent to Y*X*Z.
        Quaternion(float x, float y, float z) noexcept
        {
            FromEulerAngles(x, y, z);
        }
        /// Construct from Euler angles (in degrees).
        explicit Quaternion(const Vector3& angles) noexcept
        {
            FromEulerAngles(angles.x_, angles.y_, angles.z_);
        }
        /// Multiply with a scalar
        Quaternion operator*(float rhs) const
        {
            return Quaternion(w_ * rhs, x_ * rhs, y_ * rhs, z_ * rhs);
        }
        /// Return negation.
        Quaternion operator -() const
        {
            return Quaternion(-w_, -x_, -y_, -z_);
        }
        /// Add a quaternion.
        Quaternion operator +(const Quaternion& rhs) const
        {
            return Quaternion(w_ + rhs.w_, x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_);
        }
        /// Subtract a quaternion.
        Quaternion operator -(const Quaternion& rhs) const
        {
            return Quaternion(w_ - rhs.w_, x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_);
        }
        /// Test for equality with another quaternion without epsilon.
        bool operator ==(const Quaternion& rhs) const
        {
            return w_ == rhs.w_ && x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_;
        }
        /// Test for inequality with another quaternion without epsilon.
        bool operator !=(const Quaternion& rhs) const { return !(*this == rhs); }
        /// Define from an angle (in degrees) and axis
        void FromAngleAxis(float angle, const Vector3& axis);
        /// Define from Euler angles (in degree). Equivalent to Y*X*Z
        void FromEulerAngles(float x, float y, float z);
        /// Define from the rotation difference between two direction vectors
        void FromRotationTo(const Vector3& start, const Vector3& end);
        /// Define from orthonormal axes
        void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
        /// Define from a rotation matrix
        void FromRotaionMatrix(const Matrix3& matrix);
        /// Define from a direction to look in and an up direction.
        bool FromLookRotation(const Vector3& direction, const Vector3& up = Vector3::UP);
        /// Normalize to unit length
        void Normalize()
        {
            float lenSquared = LengthSquared();
            if (!My3D::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                w_ *= invLen;
                x_ *= invLen;
                y_ *= invLen;
                z_ *= invLen;
            }
        }
        /// Return normalized to unit length
        Quaternion Normalized() const
        {
            float lenSquared = LengthSquared();
            if (!My3D::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
            {
                float invLen = 1.0f / sqrtf(lenSquared);
                return *this * invLen;
            }
            return *this;
        }
        /// return squared length
        float LengthSquared() const
        {
            return w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
        }
        /// Calculate dot product.
        float DotProduct(const Quaternion& rhs) const
        {
            return w_ * rhs.w_ + x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_;
        }
        /// Test for equality with another quaternion with epsilon.
        bool Equals(const Quaternion& rhs) const
        {
            return My3D::Equals(w_, rhs.w_) && My3D::Equals(x_, rhs.x_) && My3D::Equals(y_, rhs.y_) && My3D::Equals(z_, rhs.z_);
        }
        /// Return whether any element is NaN.
        bool IsNaN() const { return My3D::IsNaN(w_) || My3D::IsNaN(x_) || My3D::IsNaN(y_) || My3D::IsNaN(z_); }
        /// Return whether any element is Inf.
        bool IsInf() const { return My3D::IsInf(w_) || My3D::IsInf(x_) || My3D::IsInf(y_) || My3D::IsInf(z_); }
        /// Return conjugate.
        Quaternion Conjugate() const
        {
            return Quaternion(w_, -x_, -y_, -z_);
        }
        /// Return Euler angles in degrees
        Vector3 EulerAngles() const;
        /// Return yaw angle in degrees
        float YawAngle() const;
        /// Return pitch angle in degrees
        float PitchAngle() const;
        /// Return roll angle in degrees
        float RollAngle() const;
        /// Return rotation axis.
        Vector3 Axis() const;
        /// Return rotation angle
        float Angle() const;
        /// Return rotation matrix that corresponds to this quaternion
        Matrix3 RotationMatrix() const;
        /// Special interpolation with another quaternion
        Quaternion Slerp(const Quaternion& rhs, float t) const;
        /// Normalized linear interpolation with another quaternion
        Quaternion Nlerp(const Quaternion& rhs, float t) const;
        /// Return data pointer
        const float* Data() const { return &w_; }
        /// Return as string
        String ToString() const;

        /// W coordinate
        float w_;
        /// X coordinate
        float x_;
        /// Y coordinate
        float y_;
        /// Z coordinate
        float z_;

        /// Identify quaternion
        static const Quaternion IDENTITY;
    };
}
