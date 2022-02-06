//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "Math/Rect.h"
#include "Math/Vector3.h"


namespace My3D
{
class Matrix3;
class Matrix4;
class Matrix3x4;
class Sphere;

/// Three-dimensional axis-aligned bounding box.
class MY3D_API BoundingBox
{
public:
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
    /// Define from another bounding box.
    void Define(const BoundingBox& box)
    {
        Define(box.min_, box.max_);
    }
    /// Define from a Rect.
    void Define(const Rect& rect)
    {
        Define(Vector3(rect.min_, 0.0f), Vector3(rect.max_, 0.0f));
    }
    /// Define from minimum and maximum vectors.
    void Define(const Vector3& min, const Vector3& max)
    {
        min_ = min;
        max_ = max;
    }
    /// Define from minimum and maximum floats (all dimensions same).
    void Define(float min, float max)
    {
        min_ = Vector3(min, min, min);
        max_ = Vector3(max, max, max);
    }
    /// Define from a point.
    void Define(const Vector3& point)
    {
        min_ = max_ = point;
    }
    /// Merge a point.
    void Merge(const Vector3& point)
    {
        if (point.x_ < min_.x_)
            min_.x_ = point.x_;
        if (point.y_ < min_.y_)
            min_.y_ = point.y_;
        if (point.z_ < min_.z_)
            min_.z_ = point.z_;
        if (point.x_ > max_.x_)
            max_.x_ = point.x_;
        if (point.y_ > max_.y_)
            max_.y_ = point.y_;
        if (point.z_ > max_.z_)
            max_.z_ = point.z_;
    }
    /// Merge another bounding box.
    void Merge(const BoundingBox& box)
    {
        if (box.min_.x_ < min_.x_)
            min_.x_ = box.min_.x_;
        if (box.min_.y_ < min_.y_)
            min_.y_ = box.min_.y_;
        if (box.min_.z_ < min_.z_)
            min_.z_ = box.min_.z_;
        if (box.max_.x_ > max_.x_)
            max_.x_ = box.max_.x_;
        if (box.max_.y_ > max_.y_)
            max_.y_ = box.max_.y_;
        if (box.max_.z_ > max_.z_)
            max_.z_ = box.max_.z_;
    }
    /// Define from an array of vertices.
    void Define(const Vector3* vertices, unsigned count);
    /// Define from a sphere.
    void Define(const Sphere& sphere);
    /// Merge an array of vertices.
    void Merge(const Vector3* vertices, unsigned count);
    /// Merge a sphere.
    void Merge(const Sphere& sphere);
    /// Clip with another bounding box. The box can become degenerate (undefined) as a result.
    void Clip(const BoundingBox& box);
    /// Clear to undefined state.
    void Clear()
    {
        min_ = Vector3(M_INFINITY, M_INFINITY, M_INFINITY);
        max_ = Vector3(-M_INFINITY, -M_INFINITY, -M_INFINITY);
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
