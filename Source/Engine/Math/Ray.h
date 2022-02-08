//
// Created by luchu on 2022/1/9.
//

#pragma once
#include "Math/Vector3.h"


namespace My3D
{
    class BoundingBox;
    class Frustum;
    class Plane;
    class Sphere;

    /// Infinite straight line in three-dimensional space.
    class MY3D_API Ray
    {
        /// Construct a degenerate ray with zero origin and direction.
        Ray() noexcept = default;
        /// Construct from origin and direction. The direction will be normalized.
        Ray(const Vector3& origin, const Vector3& direction) noexcept
        {
            Define(origin, direction);
        }
        /// Copy-construct from another ray.
        Ray(const Ray& ray) noexcept = default;
        /// Assign from another ray.
        Ray& operator =(const Ray& rhs) noexcept = default;
        /// Check for equality with another ray.
        bool operator ==(const Ray& rhs) const { return origin_ == rhs.origin_ && direction_ == rhs.direction_; }
        /// Check for inequality with another ray.
        bool operator !=(const Ray& rhs) const { return origin_ != rhs.origin_ || direction_ != rhs.direction_; }
        /// Define from origin and direction. The direction will be normalized.
        void Define(const Vector3& origin, const Vector3& direction)
        {
            origin_ = origin;
            direction_ = direction.Normalized();
        }
        /// Project a point on the ray.
        Vector3 Project(const Vector3& point) const
        {
            Vector3 offset = point - origin_;
            return origin_ + offset.DotProduct(direction_) * direction_;
        }
        /// Return distance of a point from the ray.
        float Distance(const Vector3& point) const
        {
            Vector3 projected = Project(point);
            return (point - projected).Length();
        }
        /// Return hit distance to a plane, or infinity if no hit.
        float HitDistance(const Plane& plane) const;
        /// Return hit distance to a bounding box, or infinity if no hit.
        float HitDistance(const BoundingBox& box) const;
        /// Return hit distance to a frustum, or infinity if no hit. If solidInside parameter is true (default) rays originating from inside return zero distance, otherwise the distance to the closest plane.
        float HitDistance(const Frustum& frustum, bool solidInside = true) const;
        /// Return hit distance to a sphere, or infinity if no hit.
        float HitDistance(const Sphere& sphere) const;
        /// Ray origin.
        Vector3 origin_;
        /// Ray direction.
        Vector3 direction_;
    };
}
