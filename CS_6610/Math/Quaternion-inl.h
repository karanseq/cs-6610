#include "Quaternion.h"

// Engine includes
#include "MathUtil.h"

namespace engine {
namespace math {

inline float Quaternion::w() const
{
    return w_;
}

inline void Quaternion::w(float i_w)
{
    w_ = i_w;
}

inline float Quaternion::x() const
{
    return x_;
}

inline void Quaternion::x(float i_x)
{
    x_ = i_x;
}

inline float Quaternion::y() const
{
    return y_;
}

inline void Quaternion::y(float i_y)
{
    y_ = i_y;
}

inline float Quaternion::z() const
{
    return z_;
}

inline void Quaternion::z(float i_z)
{
    z_ = i_z;
}

inline Quaternion& Quaternion::operator=(const Quaternion& i_quat)
{
    // Check against self assignment
    if (this != &i_quat)
    {
        w_ = i_quat.w_;
        x_ = i_quat.x_;
        y_ = i_quat.y_;
        z_ = i_quat.z_;
    }
    return *this;
}

inline Quaternion Quaternion::operator+(const Quaternion& i_quat) const
{
    return Quaternion(w_ + i_quat.w_, x_ + i_quat.x_, y_ + i_quat.y_, z_ + i_quat.z_);
}

inline Quaternion& Quaternion::operator+=(const Quaternion& i_quat)
{
    w_ += i_quat.w_;
    x_ += i_quat.x_;
    y_ += i_quat.y_;
    z_ += i_quat.z_;
    return *this;
}

inline Quaternion Quaternion::operator-(const Quaternion& i_quat) const
{
    return Quaternion(w_ - i_quat.w_, x_ - i_quat.x_, y_ - i_quat.y_, z_ - i_quat.z_);
}

inline Quaternion& Quaternion::operator-=(const Quaternion& i_quat)
{
    w_ -= i_quat.w_;
    x_ -= i_quat.x_;
    y_ -= i_quat.y_;
    z_ -= i_quat.z_;
    return *this;
}

inline Quaternion Quaternion::operator*(const Quaternion& i_quat) const
{
    return Quaternion(
        (w_ * i_quat.w_) - ((x_ * i_quat.x_) + (y_ * i_quat.y_) + (z_ * i_quat.z_)),
        (w_ * i_quat.x_) + (x_ * i_quat.w_) + ((y_ * i_quat.z_) - (z_ * i_quat.y_)),
        (w_ * i_quat.y_) + (y_ * i_quat.w_) + ((z_ * i_quat.x_) - (x_ * i_quat.z_)),
        (w_ * i_quat.z_) + (z_ * i_quat.w_) + ((x_ * i_quat.y_) - (y_ * i_quat.x_)));
}

inline Quaternion& Quaternion::operator*=(const Quaternion& i_quat)
{
    w_ = (w_ * i_quat.w_) - ((x_ * i_quat.x_) + (y_ * i_quat.y_) + (z_ * i_quat.z_));
    x_ = (w_ * i_quat.x_) + (x_ * i_quat.w_) + ((y_ * i_quat.z_) - (z_ * i_quat.y_));
    y_ = (w_ * i_quat.y_) + (y_ * i_quat.w_) + ((z_ * i_quat.x_) - (x_ * i_quat.z_));
    z_ = (w_ * i_quat.z_) + (z_ * i_quat.w_) + ((x_ * i_quat.y_) - (y_ * i_quat.x_));
    return *this;
}

inline Quaternion Quaternion::operator*(float i_scale) const
{
    return Quaternion(w_ * i_scale, x_ * i_scale, y_ * i_scale, z_ * i_scale);
}

inline Quaternion& Quaternion::operator*=(float i_scale)
{
    w_ *= i_scale;
    x_ *= i_scale;
    y_ *= i_scale;
    z_ *= i_scale;
    return *this;
}

inline bool Quaternion::operator==(const Quaternion& i_quat) const
{
    return FuzzyEquals(w_, i_quat.w_) && FuzzyEquals(x_, i_quat.x_) && FuzzyEquals(y_, i_quat.y_) && FuzzyEquals(z_, i_quat.z_);
}

inline bool Quaternion::operator!=(const Quaternion& i_quat) const
{
    return !FuzzyEquals(w_, i_quat.w_) || !FuzzyEquals(x_, i_quat.x_) || !FuzzyEquals(y_, i_quat.y_) || !FuzzyEquals(z_, i_quat.z_);
}

} // namespace math
} // namespace engine
