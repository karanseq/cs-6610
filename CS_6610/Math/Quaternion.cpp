#include "Quaternion.h"

// Library includes
#include <cmath>

// Engine includes
#include "MathUtil.h"
#include "Vec3D.h"

namespace engine {
namespace math {

const Quaternion Quaternion::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f);
const Quaternion Quaternion::FORWARD(0.0f, 0.0f, 0.0f, 1.0f);
const Quaternion Quaternion::UP(0.0f, 0.0f, 1.0f, 0.0f);
const Quaternion Quaternion::RIGHT(0.0f, 1.0f, 0.0f, 0.0f);

Quaternion::Quaternion(float i_angle_radians, const Vec3D& i_axis)
{
    const float theta_half = i_angle_radians * 0.5f;
    w_ = std::cos(theta_half);
    const float sin_theta_half = std::sin(theta_half);
    i_axis.Normalize();
    x_ = i_axis.x() * sin_theta_half;
    y_ = i_axis.y() * sin_theta_half;
    z_ = i_axis.z() * sin_theta_half;
}

Quaternion::Quaternion(float i_w, float i_x, float i_y, float i_z) : w_(i_w),
    x_(i_x),
    y_(i_y),
    z_(i_z)
{}

Quaternion::Quaternion(const Quaternion& i_quat) : w_(i_quat.w_),
    x_(i_quat.x_),
    y_(i_quat.y_),
    z_(i_quat.z_)
{}

void Quaternion::Invert()
{
    x_ = -x_;
    y_ = -y_;
    z_ = -z_;
}

Quaternion Quaternion::GetInverse() const
{
    return Quaternion(w_, -x_, -y_, -z_);
}

void Quaternion::Normalize()
{
    const float length = std::sqrt(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_);
    const float length_reciprocal = 1.0f / length;
    w_ *= length_reciprocal;
    x_ *= length_reciprocal;
    y_ *= length_reciprocal;
    z_ *= length_reciprocal;
}

Quaternion Quaternion::GetNormalized() const
{
    const float length = std::sqrt(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_);
    const float length_reciprocal = 1.0f / length;
    return Quaternion(w_ * length_reciprocal, x_ * length_reciprocal, y_ * length_reciprocal, z_ * length_reciprocal);
}

Vec3D Quaternion::GetForwardVector() const
{
    const float _2x = x_ + x_;
    const float _2y = y_ + y_;
    const float _2xx = x_ * _2x;
    const float _2xz = _2x * z_;
    const float _2xw = _2x * w_;
    const float _2yy = _2y * y_;
    const float _2yz = _2y * z_;
    const float _2yw = _2y * w_;

    return Vec3D(-_2xz - _2yw, -_2yz + _2xw, -1.0f + _2xx + _2yy);
}

} // namespace math
} // namespace engine
