//
// Created by luchu on 2022/1/9.
//

#pragma once
#include "Math/Vector3.h"


namespace My3D
{
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

        /// Define from origin and direction. The direction will be normalized.
        void Define(const Vector3& origin, const Vector3& direction)
        {
            origin_ = origin;
            direction_ = direction.Normalized();
        }

        /// Ray origin.
        Vector3 origin_;
        /// Ray direction.
        Vector3 direction_;
    };
}
