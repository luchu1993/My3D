//
// Created by luchu on 2022/1/9.
//

#include "Math/Sphere.h"
#include "Math/BoundingBox.h"


namespace My3D
{
    void Sphere::Define(const Vector3 *vertices, unsigned int count)
    {
        if (!count)
            return;

        Clear();
        Merge(vertices, count);
    }

    void Sphere::Define(const BoundingBox& box)
    {
        const Vector3& min = box.min_;
        const Vector3& max = box.max_;

        Clear();
        Merge(min);
        Merge(Vector3(max.x_, min.y_, min.z_));
        Merge(Vector3(min.x_, max.y_, min.z_));
        Merge(Vector3(max.x_, max.y_, min.z_));
        Merge(Vector3(min.x_, min.y_, max.z_));
        Merge(Vector3(max.x_, min.y_, max.z_));
        Merge(Vector3(min.x_, max.y_, max.z_));
        Merge(max);
    }

    void Sphere::Merge(const Vector3* vertices, unsigned count)
    {
        while (count--)
            Merge(*vertices++);
    }

    void Sphere::Merge(const BoundingBox& box)
    {
        const Vector3& min = box.min_;
        const Vector3& max = box.max_;

        Merge(min);
        Merge(Vector3(max.x_, min.y_, min.z_));
        Merge(Vector3(min.x_, max.y_, min.z_));
        Merge(Vector3(max.x_, max.y_, min.z_));
        Merge(Vector3(min.x_, min.y_, max.z_));
        Merge(Vector3(max.x_, min.y_, max.z_));
        Merge(Vector3(min.x_, max.y_, max.z_));
        Merge(max);
    }

    Intersection Sphere::IsInside(const BoundingBox& box) const
    {
        float radiusSquared = radius_ * radius_;
        float distSquared = 0;
        float temp;
        Vector3 min = box.min_;
        Vector3 max = box.max_;

        if (center_.x_ < min.x_)
        {
            temp = center_.x_ - min.x_;
            distSquared += temp * temp;
        }
        else if (center_.x_ > max.x_)
        {
            temp = center_.x_ - max.x_;
            distSquared += temp * temp;
        }
        if (center_.y_ < min.y_)
        {
            temp = center_.y_ - min.y_;
            distSquared += temp * temp;
        }
        else if (center_.y_ > max.y_)
        {
            temp = center_.y_ - max.y_;
            distSquared += temp * temp;
        }
        if (center_.z_ < min.z_)
        {
            temp = center_.z_ - min.z_;
            distSquared += temp * temp;
        }
        else if (center_.z_ > max.z_)
        {
            temp = center_.z_ - max.z_;
            distSquared += temp * temp;
        }

        if (distSquared >= radiusSquared)
            return OUTSIDE;

        min -= center_;
        max -= center_;

        Vector3 tempVec = min; // - - -
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.x_ = max.x_; // + - -
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.y_ = max.y_; // + + -
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.x_ = min.x_; // - + -
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.z_ = max.z_; // - + +
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.y_ = min.y_; // - - +
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.x_ = max.x_; // + - +
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;
        tempVec.y_ = max.y_; // + + +
        if (tempVec.LengthSquared() >= radiusSquared)
            return INTERSECTS;

        return INSIDE;
    }

    Intersection Sphere::IsInsideFast(const BoundingBox& box) const
    {
        float radiusSquared = radius_ * radius_;
        float distSquared = 0;
        float temp;
        Vector3 min = box.min_;
        Vector3 max = box.max_;

        if (center_.x_ < min.x_)
        {
            temp = center_.x_ - min.x_;
            distSquared += temp * temp;
        }
        else if (center_.x_ > max.x_)
        {
            temp = center_.x_ - max.x_;
            distSquared += temp * temp;
        }
        if (center_.y_ < min.y_)
        {
            temp = center_.y_ - min.y_;
            distSquared += temp * temp;
        }
        else if (center_.y_ > max.y_)
        {
            temp = center_.y_ - max.y_;
            distSquared += temp * temp;
        }
        if (center_.z_ < min.z_)
        {
            temp = center_.z_ - min.z_;
            distSquared += temp * temp;
        }
        else if (center_.z_ > max.z_)
        {
            temp = center_.z_ - max.z_;
            distSquared += temp * temp;
        }

        if (distSquared >= radiusSquared)
            return OUTSIDE;
        else
            return INSIDE;
    }

    Vector3 Sphere::GetLocalPoint(float theta, float phi) const
    {
        return Vector3(
                radius_ * Sin(theta) * Sin(phi),
                radius_ * Cos(phi),
                radius_ * Cos(theta) * Sin(phi)
        );
    }
}