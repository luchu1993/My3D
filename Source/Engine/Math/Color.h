//
// Created by luchu on 2022/1/9.
//

#pragma once

#include "My3D.h"
#include "Container/String.h"
#include "Vector4.h"


namespace My3D {

class MY3D_API Color {
public:
    /// Construct with default values (opaque white).
    Color() noexcept: r_(1.0f), g_(1.0f), b_(1.0f), a_(1.0f) {}
    /// Copy-construct from another color.
    Color(const Color &color) noexcept = default;
    /// Construct from another color and modify the alpha.
    Color(const Color &color, float a) noexcept
        : r_(color.r_)
        , g_(color.g_)
        , b_(color.b_)
        , a_(a)
    {
    }
    /// Construct from RGB values and set alpha fully opaque.
    Color(float r, float g, float b) noexcept
        : r_(r)
        , g_(g)
        , b_(b)
        , a_(1.0f)
    {
    }
    /// Construct from RGBA values.
    Color(float r, float g, float b, float a) noexcept
        : r_(r)
        , g_(g)
        , b_(b)
        , a_(a)
    {
    }
    /// Construct from a float array.
    explicit Color(const float *data) noexcept
        : r_(data[0])
        , g_(data[1])
        , b_(data[2])
        , a_(data[3])
    {
    }
    /// Test for equality with another color without epsilon.
    bool operator ==(const Color &rhs) const { return r_ == rhs.r_ && g_ == rhs.g_ && b_ == rhs.b_ && a_ == rhs.a_; }
    /// Test for inequality with another color without epsilon.
    bool operator !=(const Color &rhs) const { return r_ != rhs.r_ || g_ != rhs.g_ || b_ != rhs.b_ || a_ != rhs.a_; }
    /// Multiply with a scalar.
    Color operator *(float rhs) const { return Color(r_ * rhs, g_ * rhs, b_ * rhs, a_ * rhs); }
    /// Add a color.
    Color operator +(const Color& rhs) const { return Color(r_ + rhs.r_, g_ + rhs.g_, b_ + rhs.b_, a_ + rhs.a_); }
    /// Return negation.
    Color operator -() const { return Color(-r_, -g_, -b_, -a_); }
    /// Subtract a color.
    Color operator -(const Color& rhs) const { return Color(r_ - rhs.r_, g_ - rhs.g_, b_ - rhs.b_, a_ - rhs.a_); }
    /// Add-assign a color.
    Color& operator +=(const Color& rhs)
    {
        r_ += rhs.r_;
        g_ += rhs.g_;
        b_ += rhs.b_;
        a_ += rhs.a_;
        return *this;
    }
    /// Return float data.
    const float* Data() const { return &r_; }
    /// Clip to [0, 1.0] range.
    void Clip(bool clipAlpha = false);
    /// Inverts the RGB channels and optionally the alpha channel as well.
    void Invert(bool invertAlpha = false);
    /// Return linear interpolation of this color with another color.
    Color Lerp(const Color& rhs, float t) const;
    /// Return color with absolute components.
    Color Abs() const { return Color(My3D::Abs(r_), My3D::Abs(g_), My3D::Abs(b_), My3D::Abs(a_)); }
    /// Test for equality with another color with epsilon.
    bool Equals(const Color& rhs) const
    {
        return My3D::Equals(r_, rhs.r_) && My3D::Equals(g_, rhs.g_) && My3D::Equals(b_, rhs.b_) && My3D::Equals(a_, rhs.a_);
    }
    /// Return as string.
    String ToString() const;

    /// Red value
    float r_;
    /// Green value
    float g_;
    /// Blue value
    float b_;
    /// Alpha value
    float a_;

    /// Opaque white color
    static const Color WHITE;
    /// Opaque gray color
    static const Color GRAY;
    /// Opaque black color
    static const Color BLACK;
    /// Opaque red color
    static const Color RED;
    /// Opaque green color
    static const Color GREEN;
    /// Opaque blue color
    static const Color BLUE;
    /// Opaque cyan color
    static const Color CYAN;
    /// Opaque magenta color.
    static const Color MAGENTA;
    /// Opaque yellow color.
    static const Color YELLOW;
    /// Transparent black color (black with no alpha).
    static const Color TRANSPARENT_BLACK;
};

}