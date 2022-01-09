//
// Created by luchu on 2022/1/9.
//

#include "Math/Color.h"

namespace My3D
{
    const Color Color::WHITE;
    const Color Color::GRAY(0.5f, 0.5f, 0.5f);
    const Color Color::BLACK(0.0f, 0.0f, 0.0f);
    const Color Color::RED(1.0f, 0.0f, 0.0f);
    const Color Color::GREEN(0.0f, 1.0f, 0.0f);
    const Color Color::BLUE(0.0f, 0.0f, 1.0f);
    const Color Color::CYAN(0.0f, 1.0f, 1.0f);
    const Color Color::MAGENTA(1.0f, 0.0f, 1.0f);
    const Color Color::YELLOW(1.0f, 1.0f, 0.0f);
    const Color Color::TRANSPARENT_BLACK(0.0f, 0.0f, 0.0f, 0.0f);

    String Color::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", r_, g_, b_, a_);
        return String(tempBuffer);
    }
}

