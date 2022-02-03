//
// Created by luchu on 2022/1/9.
//

#include "Math/Matrix2.h"

namespace My3D
{
    const Matrix2 Matrix2::ZERO(
            0.0f, 0.0f,
            0.0f, 0.0f);

    const Matrix2 Matrix2::IDENTITY;

    Matrix2 Matrix2::Inverse() const
    {
        float det = m00_ * m11_ - m01_ * m10_;
        float invDet = 1.0f / det;

        return Matrix2(
            m11_, -m01_,
            -m10_, m00_
        ) * invDet;
    }

    String Matrix2::ToString() const
    {
        char tempBuffer[MATRIX_CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", m00_, m01_, m10_, m11_);
        return String(tempBuffer);
    }
}
