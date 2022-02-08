//
// Created by luchu on 2022/1/9.
//

#include "Math/Ray.h"
#include "Math/Plane.h"
#include "Math/BoundingBox.h"
#include "Math/Sphere.h"
#include "Math/Frustum.h"


namespace My3D
{
    float Ray::HitDistance(const Plane& plane) const
    {
        float d = plane.normal_.DotProduct(direction_);
        if (Abs(d) >= M_EPSILON)
        {
            float t = -(plane.normal_.DotProduct(origin_) + plane.d_) / d;
            if (t >= 0.0f)
                return t;
            else
                return M_INFINITY;
        }
        else
            return M_INFINITY;
    }

    float Ray::HitDistance(const BoundingBox& box) const
    {
        // If undefined, no hit (infinite distance)
        if (!box.Defined())
            return M_INFINITY;

        // Check for ray origin being inside the box
        if (box.IsInside(origin_))
            return 0.0f;

        float dist = M_INFINITY;

        // Check for intersecting in the X-direction
        if (origin_.x_ < box.min_.x_ && direction_.x_ > 0.0f)
        {
            float x = (box.min_.x_ - origin_.x_) / direction_.x_;
            if (x < dist)
            {
                Vector3 point = origin_ + x * direction_;
                if (point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                    dist = x;
            }
        }
        if (origin_.x_ > box.max_.x_ && direction_.x_ < 0.0f)
        {
            float x = (box.max_.x_ - origin_.x_) / direction_.x_;
            if (x < dist)
            {
                Vector3 point = origin_ + x * direction_;
                if (point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                    dist = x;
            }
        }
        // Check for intersecting in the Y-direction
        if (origin_.y_ < box.min_.y_ && direction_.y_ > 0.0f)
        {
            float x = (box.min_.y_ - origin_.y_) / direction_.y_;
            if (x < dist)
            {
                Vector3 point = origin_ + x * direction_;
                if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                    dist = x;
            }
        }
        if (origin_.y_ > box.max_.y_ && direction_.y_ < 0.0f)
        {
            float x = (box.max_.y_ - origin_.y_) / direction_.y_;
            if (x < dist)
            {
                Vector3 point = origin_ + x * direction_;
                if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                    dist = x;
            }
        }
        // Check for intersecting in the Z-direction
        if (origin_.z_ < box.min_.z_ && direction_.z_ > 0.0f)
        {
            float x = (box.min_.z_ - origin_.z_) / direction_.z_;
            if (x < dist)
            {
                Vector3 point = origin_ + x * direction_;
                if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_)
                    dist = x;
            }
        }
        if (origin_.z_ > box.max_.z_ && direction_.z_ < 0.0f)
        {
            float x = (box.max_.z_ - origin_.z_) / direction_.z_;
            if (x < dist)
            {
                Vector3 point = origin_ + x * direction_;
                if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_)
                    dist = x;
            }
        }

        return dist;
    }

    float Ray::HitDistance(const Frustum& frustum, bool solidInside) const
    {
        float maxOutside = 0.0f;
        float minInside = M_INFINITY;
        bool allInside = true;

        for (const auto& plane : frustum.planes_)
        {
            float distance = HitDistance(plane);

            if (plane.Distance(origin_) < 0.0f)
            {
                maxOutside = Max(maxOutside, distance);
                allInside = false;
            }
            else
                minInside = Min(minInside, distance);
        }

        if (allInside)
            return solidInside ? 0.0f : minInside;
        else if (maxOutside <= minInside)
            return maxOutside;
        else
            return M_INFINITY;
    }

    float Ray::HitDistance(const Sphere& sphere) const
    {
        Vector3 centeredOrigin = origin_ - sphere.center_;
        float squaredRadius = sphere.radius_ * sphere.radius_;

        // Check if ray originates inside the sphere
        if (centeredOrigin.LengthSquared() <= squaredRadius)
            return 0.0f;

        // Calculate intersection by quadratic equation
        float a = direction_.DotProduct(direction_);
        float b = 2.0f * centeredOrigin.DotProduct(direction_);
        float c = centeredOrigin.DotProduct(centeredOrigin) - squaredRadius;
        float d = b * b - 4.0f * a * c;

        // No solution
        if (d < 0.0f)
            return M_INFINITY;

        // Get the nearer solution
        float dSqrt = sqrtf(d);
        float dist = (-b - dSqrt) / (2.0f * a);
        if (dist >= 0.0f)
            return dist;
        else
            return (-b + dSqrt) / (2.0f * a);
    }
}
