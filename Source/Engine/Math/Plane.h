//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Math/Matrix3x4.h"


namespace My3D
{

/// Surface in three-dimensional space.
class MY3D_API Plane
{
public:
    /// Construct a degenerate plane with zero normal and parameter.
    Plane() noexcept : d_(0.0f) { }
    /// Copy-construct from another plane.
    Plane(const Plane& plane) noexcept = default;
    /// Construct from 3 vertices.
    Plane(const Vector3& v0, const Vector3& v1, const Vector3& v2) noexcept
    {
        Define(v0, v1, v2);
    }
    /// Construct from a normal vector and a point on the plane.
    Plane(const Vector3& normal, const Vector3& point) noexcept
    {
        Define(normal, point);
    }
    /// Construct from a 4-dimensional vector, where the w coordinate is the plane parameter.
    explicit Plane(const Vector4& plane) noexcept
    {
        Define(plane);
    }
    /// Assign from another plane.
    Plane& operator =(const Plane& rhs) noexcept = default;
    /// Define from 3 vertices.
    void Define(const Vector3& v0, const Vector3& v1, const Vector3& v2)
    {
        Vector3 dist1 = v1 - v0;
        Vector3 dist2 = v2 - v0;

        Define(dist1.CrossProduct(dist2), v0);
    }

    /// Define from a normal vector and a point on the plane.
    void Define(const Vector3& normal, const Vector3& point)
    {
        normal_ = normal.Normalized();
        absNormal_ = normal_.Abs();
        d_ = -normal_.DotProduct(point);
    }

    /// Define from a 4-dimensional vector, where the w coordinate is the plane parameter.
    void Define(const Vector4& plane)
    {
        normal_ = Vector3(plane.x_, plane.y_, plane.z_);
        absNormal_ = normal_.Abs();
        d_ = plane.w_;
    }

    /// Plane normal.
    Vector3 normal_;
    /// Plane absolute normal.
    Vector3 absNormal_;
    /// Plane constant.
    float d_{};
    /// Plane at origin with normal pointing up.
    static const Plane UP;
};

}
