//
// Created by luchu on 2022/1/15.
//

#include "Math/Quaternion.h"
#include "Container/String.h"


namespace My3D
{
    const Quaternion Quaternion::IDENTIFY;

    String Quaternion::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", w_, x_, y_, z_);
        return String(tempBuffer);
    }

    void Quaternion::FromAngleAxis(float angle, const Vector3 &axis)
    {
    }
}

