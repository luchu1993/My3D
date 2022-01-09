//
// Created by luchu on 2022/1/9.
//

#include "Math/Matrix3x4.h"


namespace My3D
{
    const Matrix3x4 Matrix3x4::ZERO(
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f);

    const Matrix3x4 Matrix3x4::IDENTITY;

    String Matrix3x4::ToString() const
    {
        char tempBuffer[MATRIX_CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g %g %g %g %g %g %g %g %g", m00_, m01_, m02_, m03_, m10_, m11_, m12_, m13_, m20_, m21_, m22_,
                m23_);
        return String(tempBuffer);
    }
}
