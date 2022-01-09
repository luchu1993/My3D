//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Container/String.h"
#include "Math/Vector3.h"


namespace My3D
{
class MY3D_API Matrix3
{
public:
    /// Construct an identify and scaling
    Matrix3() noexcept
        : m00_(1.0f)
        , m01_(0.0f)
        , m02_(0.0f)
        , m10_(0.0f)
        , m11_(1.0f)
        , m12_(0.0f)
        , m20_(0.0f)
        , m21_(0.0f)
        , m22_(1.0f)
    {
    }

    /// Copy-construct from another matrix.
    Matrix3(const Matrix3& matrix) noexcept = default;

    /// Construct from values.
    Matrix3(float v00, float v01, float v02,
            float v10, float v11, float v12,
            float v20, float v21, float v22) noexcept
            : m00_(v00)
            , m01_(v01)
            , m02_(v02)
            , m10_(v10)
            , m11_(v11)
            , m12_(v12)
            , m20_(v20)
            , m21_(v21)
            , m22_(v22)
    {
    }
    /// Construct from a float array.
    explicit Matrix3(const float* data) noexcept
        : m00_(data[0])
        , m01_(data[1])
        , m02_(data[2])
        , m10_(data[3])
        , m11_(data[4])
        , m12_(data[5])
        , m20_(data[6])
        , m21_(data[7])
        , m22_(data[8])
    {
    }

    /// Assign from another matrix.
    Matrix3& operator =(const Matrix3& rhs) noexcept = default;

    /// Test for equality with another matrix without epsilon.
    bool operator ==(const Matrix3& rhs) const
    {
        const float* leftData = Data();
        const float* rightData = rhs.Data();

        for (unsigned i = 0; i < 9; ++i)
        {
            if (leftData[i] != rightData[i])
                return false;
        }

        return true;
    }
    /// Return inverse.
    Matrix3 Inverse() const;
    /// Return float data.
    const float* Data() const { return &m00_; }
    /// Return as string.
    String ToString() const;

    float m00_;
    float m01_;
    float m02_;
    float m10_;
    float m11_;
    float m12_;
    float m20_;
    float m21_;
    float m22_;

    /// Zero matrix.
    static const Matrix3 ZERO;
    /// Identity matrix.
    static const Matrix3 IDENTITY;
};
}
