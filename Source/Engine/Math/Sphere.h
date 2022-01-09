//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Math/Vector3.h"


namespace My3D
{
/// Sphere in three-dimensional space.
class MY3D_API Sphere
{
    /// Construct undefined.
    Sphere() noexcept
        : center_(Vector3::ZERO)
        , radius_(-M_INFINITY)
    {
    }
    /// Copy-construct from another sphere.
    Sphere(const Sphere& sphere) noexcept = default;
    /// Construct from center and radius.
    Sphere(const Vector3& center, float radius) noexcept
        : center_(center)
        , radius_(radius)
    {
    }
    /// Clear to undefined state.
    void Clear()
    {
        center_ = Vector3::ZERO;
        radius_ = -M_INFINITY;
    }
    /// Assign from another sphere.
    Sphere& operator =(const Sphere& rhs) noexcept = default;
    /// Test for equality with another sphere.
    bool operator ==(const Sphere& rhs) const { return center_ == rhs.center_ && radius_ == rhs.radius_; }
    /// Test for inequality with another sphere.
    bool operator !=(const Sphere& rhs) const { return center_ != rhs.center_ || radius_ != rhs.radius_; }

    /// Sphere center.
    Vector3 center_;
    /// Sphere radius.
    float radius_{};
};

}
