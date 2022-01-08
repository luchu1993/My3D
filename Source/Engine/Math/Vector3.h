//
// Created by luchu on 2022/1/5.
//

#pragma once

#include "My3D.h"
#include "Math/Vector2.h"

namespace My3D
{
    class MY3D_API Vector3
    {
    public:
        /// Construct a zero vector
        Vector3() noexcept : x_(0.0f), y_(0.0f), z_(0.0f) {}
        /// Construct from another vector
        Vector3(const Vector3& vector3) = default;
        /// Construct from a two-dimensional vector and Z coordinate.
        Vector3(const Vector2& vector, float z) noexcept
            : x_(vector.x_), y_(vector.y_), z_(z) { }
        /// Construct from a two-dimensional vector
        Vector3(const Vector2& vector) noexcept
             : x_(vector.x_), y_(vector.y_), z_(0.0f) { }
        /// Construct from coordinates.
        Vector3(float x, float y, float z) noexcept : x_(x), y_(y),z_(z) { }
        /// Construct from two-dimensional coordinates (for Urho2D).
        Vector3(float x, float y) noexcept : x_(x), y_(y), z_(0.0f) { }
        /// Construct from a float array.
        explicit Vector3(const float* data) noexcept : x_(data[0]), y_(data[1]), z_(data[2]) { }

        /// X coordinate.
        float x_;
        /// Y coordinate.
        float y_;
        /// Z coordinate.
        float z_;
    };
}
