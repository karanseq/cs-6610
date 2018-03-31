#ifndef ENGINE_QUATERNION_H_
#define ENGINE_QUATERNION_H_

namespace engine {
namespace math {

// Forward declarations
class Vec3D;

/*
Quaternion
- A class that represents a rotation or an orientation
*/

class Quaternion
{
public:
    Quaternion() = default;
    Quaternion(float i_angle_radians, const Vec3D& i_axis);
    Quaternion(float i_w, float i_x, float i_y, float i_z);
    Quaternion(const Quaternion& i_quat);

    ~Quaternion() = default;

    // accessors and mutators
    inline float w() const;
    inline void w(float i_w);
    inline float x() const;
    inline void x(float i_x);
    inline float y() const;
    inline void y(float i_y);
    inline float z() const;
    inline void z(float i_z);

    // assignment
    inline Quaternion& operator=(const Quaternion& i_quat);

    // arithmetic
    inline Quaternion operator+(const Quaternion& i_quat) const;
    inline Quaternion& operator+=(const Quaternion& i_quat);
    inline Quaternion operator-(const Quaternion& i_quat) const;
    inline Quaternion& operator-=(const Quaternion& i_quat);
    inline Quaternion operator*(const Quaternion& i_quat) const;
    inline Quaternion& operator*=(const Quaternion& i_quat);
    inline Quaternion operator*(float i_scale) const;
    inline Quaternion& operator*=(float i_scale);

    // relational
    inline bool operator==(const Quaternion& i_quat) const;
    inline bool operator!=(const Quaternion& i_quat) const;

    // functions
    void Invert();
    Quaternion GetInverse() const;
    void Normalize();
    Quaternion GetNormalized() const;
    Vec3D GetForwardVector() const;

    // constants
    static const Quaternion     IDENTITY;
    static const Quaternion     FORWARD;
    static const Quaternion     UP;
    static const Quaternion     RIGHT;

public:
    float                       w_ = 1.0f;
    float                       x_ = 0.0f;
    float                       y_ = 0.0f;
    float                       z_ = 0.0f;

}; // class Quaternion

} // namespace math
} // namespace engine

#include "Quaternion-inl.h"

#endif // ENGINE_QUATERNION_H_
