//
// Created by luchu on 2022/1/1.
//

#pragma once

#include "Math/Random.h"
#include <cmath>
#include <limits>


namespace My3D
{

#undef M_PI
static const float M_PI = 3.14159265358979323846264338327950288f;
static const float M_HALF_PI = M_PI * 0.5f;
static const int M_MIN_INT = 0x80000000;
static const int M_MAX_INT = 0x7fffffff;
static const short M_MIN_SHORT = 0x8000;
static const short M_MAX_SHORT = 0x7fff;
static const unsigned M_MIN_UNSIGNED = 0x00000000;
static const unsigned M_MAX_UNSIGNED = 0xffffffff;

static const float M_EPSILON = 0.000001f;
static const float M_LARGE_EPSILON = 0.00005f;
static const float M_MIN_NEARCLIP = 0.01f;
static const float M_MAX_FOV = 160.0f;
static const float M_LARGE_VALUE = 100000000.0f;
static const float M_INFINITY = (float)HUGE_VAL;
static const float M_DEGTORAD = M_PI / 180.0f;
static const float M_DEGTORAD_2 = M_PI / 360.0f;    // M_DEGTORAD / 2.f
static const float M_RADTODEG = 1.0f / M_DEGTORAD;

/// Intersection test result.
enum Intersection
{
    OUTSIDE,
    INTERSECTS,
    INSIDE
};

/// Check whether two floating point values are equal within accuracy.
template <typename T>
inline bool Equals(T lhs, T rhs)
{
    return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs;
}
/// Linear interpolation between two values.
template <class T, class U>
inline T Lerp(T lhs, T rhs, U t) { return lhs * (1.0 - t) + rhs * t; }
/// Return the smaller of two values.
template <class T, class U>
inline T Min(T lhs, U rhs) { return lhs < rhs ? lhs : rhs; }
/// Return the larger of two values.
template <class T, class U>
inline T Max(T lhs, U rhs) { return lhs > rhs ? lhs : rhs; }
/// Return absolute value of a value.
template <class T>
inline T Abs(T value) { return value >= 0.0 ? value : -value; }
/// Return the sign of a float (-1, 0 or 1).
template <class T>
inline T Sign(T value) { return value > 0.0 ? 1.0 : (value < 0.0 ? -1.0 : 0.0); }
/// Convert degrees to radians.
template <class T>
inline T ToRadians(const T degrees) { return M_DEGTORAD * degrees; }
/// Convert radians to degrees.
template <class T>
inline T ToDegrees(const T radians) { return M_RADTODEG * radians; }
/// Check whether a floating point value is NaN.
template <class T> inline bool IsNaN(T value) { return std::isnan(value); }
/// Check whether a floating point value is positive or negative infinity.
template <class T> inline bool IsInf(T value) { return std::isinf(value); }
/// Clamp a number to a range.
template <class T>
inline T Clamp(T value, T min, T max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}
/// Smoothly damp between values.
template <class T>
inline T SmoothStep(T lhs, T rhs, T t)
{
    t = Clamp((t - lhs) / (rhs - lhs), T(0.0), T(1.0)); // Saturate t
    return t * t * (3.0 - 2.0 * t);
}
/// Return sine of an angle in degrees.
template <class T> inline T Sin(T angle) { return sin(angle * M_DEGTORAD); }
/// Return cosine of an angle in degrees.
template <class T> inline T Cos(T angle) { return cos(angle * M_DEGTORAD); }
/// Return tangent of an angle in degrees.
template <class T> inline T Tan(T angle) { return tan(angle * M_DEGTORAD); }
/// Return arc sine in degrees.
template <class T> inline T Asin(T x) { return M_RADTODEG * asin(Clamp(x, T(-1.0), T(1.0))); }
/// Return arc cosine in degrees.
template <class T> inline T Acos(T x) { return M_RADTODEG * acos(Clamp(x, T(-1.0), T(1.0))); }
/// Return arc tangent in degrees.
template <class T> inline T Atan(T x) { return M_RADTODEG * atan(x); }
/// Return arc tangent of y/x in degrees.
template <class T> inline T Atan2(T y, T x) { return M_RADTODEG * atan2(y, x); }
/// Return X in power Y.
template <class T> inline T Pow(T x, T y) { return pow(x, y); }
/// Return natural logarithm of X.
template <class T> inline T Ln(T x) { return log(x); }
/// Return square root of X.
template <class T> inline T Sqrt(T x) { return sqrt(x); }

/// Return fractional part of passed value in range [0, 1).
template <class T> inline T Fract(T value) { return value - floor(value); }
/// Round value down.
template <class T> inline T Floor(T x) { return floor(x); }
/// Round value down. Returns integer value.
template <class T> inline int FloorToInt(T x) { return static_cast<int>(floor(x)); }
/// Round value to nearest integer.
template <class T> inline T Round(T x) { return round(x); }
/// Round value to nearest integer.
template <class T> inline int RoundToInt(T x) { return static_cast<int>(round(x)); }
/// Round value up.
template <class T> inline T Ceil(T x) { return ceil(x); }
/// Round value up.
template <class T> inline int CeilToInt(T x) { return static_cast<int>(ceil(x)); }
/// Return a representation of the specified floating-point value as a single format bit layout.
inline unsigned FloatToRawIntBits(float value)
{
    unsigned u = *((unsigned*)&value);
    return u;
}
/// Check whether an unsigned integer is a power of two.
inline bool IsPowerOfTwo(unsigned value)
{
    return !(value & (value - 1)) && value;
}
/// Round up to next power of two.
inline unsigned NextPowerOfTwo(unsigned value)
{
    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    --value;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return ++value;
}

/// Update a hash with the given 8-bit value using the SDBM algorithm.
inline constexpr unsigned SDBMHash(unsigned hash, unsigned char c) { return c + (hash << 6u) + (hash << 16u) - hash; }
/// Return a random float between 0.0 (inclusive) and 1.0 (exclusive).
inline float Random() { return Rand() / 32768.0f; }
/// Return a random float between 0.0 and range, inclusive from both ends.
inline float Random(float range) { return Rand() * range / 32767.0f; }
/// Return a random float between min and max, inclusive from both ends.
inline float Random(float min, float max) { return Rand() * (max - min) / 32767.0f + min; }
/// Return a random integer between 0 and range - 1.
inline int Random(int range) { return (int)(Random() * range); }
/// Return a random integer between min and max - 1.
inline int Random(int min, int max) { auto range = (float)(max - min); return (int)(Random() * range) + min; }
/// Return a random normal distributed number with the given mean value and variance.
inline float RandomNormal(float meanValue, float variance) { return RandStandardNormal() * sqrtf(variance) + meanValue; }

}