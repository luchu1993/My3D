//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Container/String.h"
#include "Math/Matrix3.h"
#include "Math/Vector4.h"


namespace My3D
{
    class MY3D_API Matrix3x4
    {
    public:
        // Construct an identity matrix
        Matrix3x4() noexcept
            : m00_(1.0f)
            , m01_(0.0f)
            , m02_(0.0f)
            , m03_(0.0f)
            , m10_(0.0f)
            , m11_(1.0f)
            , m12_(0.0f)
            , m13_(0.0f)
            , m20_(0.0f)
            , m21_(0.0f)
            , m22_(1.0f)
            , m23_(0.0f)
        {
        }
        /// Copy-construct from another matrix.
        Matrix3x4(const Matrix3x4& matrix) noexcept = default;
        /// Copy-construct from a 3x3 matrix and set the extra elements to identity.
        explicit Matrix3x4(const Matrix3& matrix) noexcept
            : m00_(matrix.m00_)
            , m01_(matrix.m01_)
            , m02_(matrix.m02_)
            , m03_(0.0f)
            , m10_(matrix.m10_)
            , m11_(matrix.m11_)
            , m12_(matrix.m12_)
            , m13_(0.0f)
            , m20_(matrix.m20_)
            , m21_(matrix.m21_)
            , m22_(matrix.m22_)
            , m23_(0.0f)
        {
        }
        /// Construct from values.
        Matrix3x4(float v00, float v01, float v02, float v03,
                  float v10, float v11, float v12, float v13,
                  float v20, float v21, float v22, float v23) noexcept
            : m00_(v00)
            , m01_(v01)
            , m02_(v02)
            , m03_(v03)
            , m10_(v10)
            , m11_(v11)
            , m12_(v12)
            , m13_(v13)
            , m20_(v20)
            , m21_(v21)
            , m22_(v22)
            , m23_(v23)
        {
        }

        /// Construct from a float array.
        explicit Matrix3x4(const float* data) noexcept
            : m00_(data[0])
            , m01_(data[1])
            , m02_(data[2])
            , m03_(data[3])
            , m10_(data[4])
            , m11_(data[5])
            , m12_(data[6])
            , m13_(data[7])
            , m20_(data[8])
            , m21_(data[9])
            , m22_(data[10])
            , m23_(data[11])
        {
        }
        /// Assign from another matrix.
        Matrix3x4& operator =(const Matrix3x4& rhs) noexcept = default;

        /// Test for equality with another matrix without epsilon.
        bool operator ==(const Matrix3x4& rhs) const
        {
            const float* leftData = Data();
            const float* rightData = rhs.Data();

            for (unsigned i = 0; i < 12; ++i)
            {
                if (leftData[i] != rightData[i])
                    return false;
            }

            return true;
        }
        /// Test for inequality with another matrix without epsilon.
        bool operator !=(const Matrix3x4& rhs) const { return !(*this == rhs); }
        /// Return float data.
        const float* Data() const { return &m00_; }
        /// Return as string.
        String ToString() const;

        float m00_;
        float m01_;
        float m02_;
        float m03_;
        float m10_;
        float m11_;
        float m12_;
        float m13_;
        float m20_;
        float m21_;
        float m22_;
        float m23_;

        /// Zero matrix.
        static const Matrix3x4 ZERO;
        /// Identity matrix.
        static const Matrix3x4 IDENTITY;
    };
}
