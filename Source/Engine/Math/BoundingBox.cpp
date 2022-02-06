//
// Created by luchu on 2022/1/9.
//


#include "Math/BoundingBox.h"
#include "Math/Sphere.h"


namespace My3D
{
    void BoundingBox::Define(const Vector3 *vertices, unsigned count)
    {
        Clear();

        if (!count)
            return;

        Merge(vertices, count);
    }

    void BoundingBox::Define(const Sphere &sphere)
    {
        const Vector3& center = sphere.center_;
        float radius = sphere.radius_;

        min_ = center + Vector3(-radius, -radius, -radius);
        max_ = center + Vector3(radius, radius, radius);
    }

    void BoundingBox::Merge(const Vector3 *vertices, unsigned count)
    {
        while (count--)
            Merge(*vertices++);
    }

    void BoundingBox::Merge(const Sphere& sphere)
    {
        const Vector3& center = sphere.center_;
        float radius = sphere.radius_;

        Merge(center + Vector3(radius, radius, radius));
        Merge(center + Vector3(-radius, -radius, -radius));
    }

    void BoundingBox::Clip(const BoundingBox& box)
    {
        if (box.min_.x_ > min_.x_)
            min_.x_ = box.min_.x_;
        if (box.max_.x_ < max_.x_)
            max_.x_ = box.max_.x_;
        if (box.min_.y_ > min_.y_)
            min_.y_ = box.min_.y_;
        if (box.max_.y_ < max_.y_)
            max_.y_ = box.max_.y_;
        if (box.min_.z_ > min_.z_)
            min_.z_ = box.min_.z_;
        if (box.max_.z_ < max_.z_)
            max_.z_ = box.max_.z_;

        if (min_.x_ > max_.x_ || min_.y_ > max_.y_ || min_.z_ > max_.z_)
        {
            min_ = Vector3(M_INFINITY, M_INFINITY, M_INFINITY);
            max_ = Vector3(-M_INFINITY, -M_INFINITY, -M_INFINITY);
        }
    }

    String BoundingBox::ToString() const
    {
        return min_.ToString() + " - " + max_.ToString();
    }
}
