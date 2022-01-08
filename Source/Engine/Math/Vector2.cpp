//
// Created by luchu on 2022/1/5.
//

#include "Math/Vector2.h"
#include "Container/String.h"
#include <cstdio>


namespace My3D
{

    const Vector2 Vector2::ZERO;
    const Vector2 Vector2::LEFT(-1.0f, 0.0f);
    const Vector2 Vector2::RIGHT(1.0f, 0.0f);
    const Vector2 Vector2::UP(0.0f, 1.0f);
    const Vector2 Vector2::DOWN(0.0f, -1.0f);
    const Vector2 Vector2::ONE(1.0f, 1.0f);

    String Vector2::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g", x_, y_);
        return String(tempBuffer);
    }
}