//
// Created by luchu on 2022/1/9.
//

#include "Math/Matrix4.h"

namespace My3D
{

const Matrix4 Matrix4::ZERO(
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f);

const Matrix4 Matrix4::IDENTITY;

String Matrix4::ToString() const
{
    char tempBuffer[MATRIX_CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g", m00_, m01_, m02_, m03_, m10_, m11_, m12_, m13_, m20_,
            m21_, m22_, m23_, m30_, m31_, m32_, m33_);
    return String(tempBuffer);
}

}
