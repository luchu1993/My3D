//
// Created by luchu on 2022/1/15.
//

#include "Math/Quaternion.h"
#include "Container/String.h"


namespace My3D
{
    const Quaternion Quaternion::IDENTITY;

    String Quaternion::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", w_, x_, y_, z_);
        return String(tempBuffer);
    }

    void Quaternion::FromAngleAxis(float angle, const Vector3 &axis)
    {

    }

    void Quaternion::FromEulerAngles(float x, float y, float z)
    {

    }

    void Quaternion::FromRotationMatrix(const Matrix3 &matrix)
    {

    }

    void Quaternion::FromRotationTo(const Vector3 &start, const Vector3 &end)
    {

    }

    Quaternion Quaternion::Slerp(const Quaternion &rhs, float t) const
    {
        // Favor accuracy for native code builds
        float cosAngle = DotProduct(rhs);
        float sign = 1.0f;
        // Enable shortest path rotation
        if (cosAngle < 0.0f)
        {
            cosAngle = -cosAngle;
            sign = -1.0f;
        }

        float angle = acosf(cosAngle);
        float sinAngle = sinf(angle);
        float t1, t2;

        if (sinAngle > 0.001f)
        {
            float invSinAngle = 1.0f / sinAngle;
            t1 = sinf((1.0f - t) * angle) * invSinAngle;
            t2 = sinf(t * angle) * invSinAngle;
        }
        else
        {
            t1 = 1.0f - t;
            t2 = t;
        }

        return *this * t1 + (rhs * sign) * t2;
    }
}

