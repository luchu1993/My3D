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

    void Color::Clip(bool clipAlpha)
    {
        r_ = (r_ > 1.0f) ? 1.0f : ((r_ < 0.0f) ? 0.0f : r_);
        g_ = (g_ > 1.0f) ? 1.0f : ((g_ < 0.0f) ? 0.0f : g_);
        b_ = (b_ > 1.0f) ? 1.0f : ((b_ < 0.0f) ? 0.0f : b_);

        if (clipAlpha)
            a_ = (a_ > 1.0f) ? 1.0f : ((a_ < 0.0f) ? 0.0f : a_);
    }

    void Color::Invert(bool invertAlpha)
    {
        r_ = 1.0f - r_;
        g_ = 1.0f - g_;
        b_ = 1.0f - b_;

        if (invertAlpha)
            a_ = 1.0f - a_;
    }

    Color Color::Lerp(const Color& rhs, float t) const
    {
        float invT = 1.0f - t;
        return Color(
            r_ * invT + rhs.r_ * t,
            g_ * invT + rhs.g_ * t,
            b_ * invT + rhs.b_ * t,
            a_ * invT + rhs.a_ * t
        );
    }

    String Color::ToString() const
    {
        char tempBuffer[CONVERSION_BUFFER_LENGTH];
        sprintf(tempBuffer, "%g %g %g %g", r_, g_, b_, a_);
        return String(tempBuffer);
    }
}

