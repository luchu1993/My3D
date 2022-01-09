//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Container/String.h"
#include "Math/Vector4.h"


namespace My3D
{
/// Two-dimensional bounding rectangle.
class MY3D_API Rect
{
public:
    /// Construct an undefined rect.
    Rect() noexcept
        : min_(M_INFINITY, M_INFINITY)
        , max_(-M_INFINITY, -M_INFINITY)
    {
    }
    /// Construct from minimum and maximum vectors.
    Rect(const Vector2& min, const Vector2& max) noexcept
        : min_(min)
        , max_(max)
    {
    }
    /// Construct from coordinates.
    Rect(float left, float top, float right, float bottom) noexcept
        : min_(left, top)
        , max_(right, bottom)
    {
    }
    /// Test for equality with another rect.
    bool operator ==(const Rect& rhs) const { return min_ == rhs.min_ && max_ == rhs.max_; }
    /// Test for inequality with another rect.
    bool operator !=(const Rect& rhs) const { return min_ != rhs.min_ || max_ != rhs.max_; }
    /// Return float data.
    const float* Data() const { return &min_.x_; }
    /// Return as string.
    String ToString() const;
    /// Minimum vector.
    Vector2 min_;
    /// Maximum vector.
    Vector2 max_;

    /// Rect in the range (-1, -1) - (1, 1).
    static const Rect FULL;
    /// Rect in the range (0, 0) - (1, 1).
    static const Rect POSITIVE;
    /// Zero-sized rect.
    static const Rect ZERO;
};
}