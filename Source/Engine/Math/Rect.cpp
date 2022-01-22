//
// Created by luchu on 2022/1/9.
//
#include "Math/Rect.h"


namespace My3D
{
    const Rect Rect::FULL(-1.0f, -1.0f, 1.0f, 1.0f);
    const Rect Rect::POSITIVE(0.0f, 0.0f, 1.0f, 1.0f);
    const Rect Rect::ZERO(0.0f, 0.0f, 0.0f, 0.0f);

    const IntRect IntRect::ZERO(0, 0, 0, 0);

    String Rect::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", min_.x_, min_.y_, max_.x_, max_.y_);
        return String(tempBuffer);
    }

    String IntRect::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%d %d %d %d", left_, top_, right_, bottom_);
        return String(tempBuffer);
    }

}