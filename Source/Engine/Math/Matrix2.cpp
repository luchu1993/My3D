//
// Created by luchu on 2022/1/9.
//

#include "Math/Matrix2.h"

namespace My3D
{
    String Matrix2::ToString() const
    {
        char tempBuffer[MATRIX_CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", m00_, m01_, m10_, m11_);
        return String(tempBuffer);
    }
}
