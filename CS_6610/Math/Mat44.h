#ifndef ENGINE_MAT44_H_
#define ENGINE_MAT44_H_

#include "Vec4D.h"

namespace engine {
namespace math {

// forward declarations
class Quaternion;
class Vec3D;

class Mat44
{
public:
    explicit Mat44();
    explicit Mat44(float i_00, float i_01, float i_02, float i_03,
                   float i_10, float i_11, float i_12, float i_13,
                   float i_20, float i_21, float i_22, float i_23,
                   float i_30, float i_31, float i_32, float i_33);
    explicit Mat44(const Quaternion& i_rotation, const Vec3D& i_translation);
    ~Mat44();

    void Set(float i_00, float i_01, float i_02, float i_03,
             float i_10, float i_11, float i_12, float i_13,
             float i_20, float i_21, float i_22, float i_23,
             float i_30, float i_31, float i_32, float i_33);

    // copy constructor
    Mat44(const Mat44& i_copy);
    // copy assignment operator
    Mat44& operator=(const Mat44& i_other);

    // validity check
    inline bool IsValid() const;

    // get the determinant
    float GetDeterminant() const;

    // invert this matrix
    void Invert();
    // return an inverse of this matrix
    Mat44 GetInverse() const;

    // transpose this matrix
    void Transpose();
    // return a transpose of this matrix
    Mat44 GetTranspose() const;
    
    // multiply this matrix with a scalar
    //Mat44 operator*(float i_scalar);
    // equality check
    bool operator==(const Mat44& i_other) const;
    // validity check
    inline operator bool() const;

    // multiply a matrix with this matrix viz. this * i_right_mat
    Mat44 Multiply(const Mat44& i_right_mat) const;
    // multiply a Vec4D with this matrix viz. i_vec4 * this
    Vec4D MultiplyLeft(const Vec4D& i_left_vec4) const;
    // multiply a Vec4D with this matrix viz. this * i_vec4
    Vec4D MultiplyRight(const Vec4D& i_right_vec4) const;

    // translation matrix
    static Mat44 GetTranslation(const Vec3D& i_translation);

    // rotation matrices
    static Mat44 GetRotationX(const float i_radians);
    static Mat44 GetRotationY(const float i_radians);
    static Mat44 GetRotationZ(const float i_radians);

    // scaling matrix
    static Mat44 GetScale(const float i_scale);

#ifdef BUILD_DEBUG
    void Print() const;
#endif

    static const Mat44 IDENTITY;
    static const Mat44 ZERO;

public:
    float       m00, m01, m02, m03;
    float       m10, m11, m12, m13;
    float       m20, m21, m22, m23;
    float       m30, m31, m32, m33;

}; // class Mat44

// matrix multiplication
inline Mat44 operator*(const Mat44& i_left, const Mat44& i_right);
// V = M * V
inline Vec4D operator*(const Mat44& i_left, const Vec4D& i_right);
// V = V * M
inline Vec4D operator*(const Vec4D& i_left, const Mat44& i_right);

} // namespace math
} // namespace engine

#include "Mat44-inl.h"

#endif // ENGINE_MAT44_H_
