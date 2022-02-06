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
}