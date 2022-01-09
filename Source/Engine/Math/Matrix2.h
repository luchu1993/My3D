//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Container/String.h"
#include "Math/Vector2.h"

namespace My3D
{
    /// 2x2 matrix for rotation and scaling
    class MY3D_API Matrix2
    {
    public:
        /// Construct and identity matrix
        Matrix2() noexcept
            : m00_(1.0f)
            , m01_(0.0f)
            , m10_(0.0f)
            , m11_(1.0f)
        {
        }
        /// Copy-construct from another matrix.
        Matrix2(const Matrix2& matrix) noexcept = default;
        /// Construct from values.
        Matrix2(float v00, float v01, float v10, float v11) noexcept
            : m00_(v00)
            , m01_(v01)
            , m10_(v10)
            , m11_(v11)
        {
        }
        /// Construct from a float array.
        explicit Matrix2(const float* data) noexcept
            : m00_(data[0])
            , m01_(data[1])
            , m10_(data[2])
            , m11_(data[3])
        {
        }
        /// Assign from another matrix.
        Matrix2& operator =(const Matrix2& rhs) noexcept = default;
        /// Return as string
        String ToString() const;

        float m00_;
        float m01_;
        float m10_;
        float m11_;

    };
}
