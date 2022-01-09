//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Math/Rect.h"
#include "Math/Vector3.h"


namespace My3D
{
/// Three-dimensional axis-aligned bounding box.
class MY3D_API BoundingBox
{
    /// Construct with zero size.
    BoundingBox() noexcept
        : min_(M_INFINITY, M_INFINITY, M_INFINITY)
        , max_(-M_INFINITY, -M_INFINITY, -M_INFINITY)
    {
    }
    /// Copy-construct from another bounding box.
    BoundingBox(const BoundingBox& box) noexcept
        : min_(box.min_)
        , max_(box.max_)
    {
    }
    /// Construct from a rect, with the Z dimension left zero.
    explicit BoundingBox(const Rect& rect) noexcept
        : min_(Vector3(rect.min_, 0.0f))
        , max_(Vector3(rect.max_, 0.0f))
    {
    }
    /// Construct from minimum and maximum vectors.
    BoundingBox(const Vector3& min, const Vector3& max) noexcept
        : min_(min)
        , max_(max)
    {
    }
    /// Construct from minimum and maximum floats (all dimensions same).
    BoundingBox(float min, float max) noexcept
        : min_(Vector3(min, min, min))
        , max_(Vector3(max, max, max))
    {
    }
    /// Return as string.
    String ToString() const;

    /// Minimum vector.
    Vector3 min_;
    float dummyMin_{}; // This is never used, but exists to pad the min_ value to four floats.
    /// Maximum vector.
    Vector3 max_;
    float dummyMax_{}; // This is never used, but exists to pad the max_ value to four floats.
};

}
