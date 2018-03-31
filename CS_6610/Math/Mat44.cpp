#include "Mat44.h"

// library includes
#include <math.h>

// engine includes
#include "MathUtil.h"
#include "Quaternion.h"
#include "Vec3D.h"
#include "Vec4D.h"

namespace engine {
namespace math {

const Mat44 Mat44::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);

const Mat44 Mat44::ZERO(0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f, 
                        0.0f, 0.0f, 0.0f, 0.0f, 
                        0.0f, 0.0f, 0.0f, 0.0f);

Mat44::Mat44()
{
#ifdef BUILD_DEBUG
    m00 = NAN;
#endif
}

Mat44::Mat44(float i_00, float i_01, float i_02, float i_03,
             float i_10, float i_11, float i_12, float i_13,
             float i_20, float i_21, float i_22, float i_23,
             float i_30, float i_31, float i_32, float i_33) :
    m00(i_00), m01(i_01), m02(i_02), m03(i_03),
    m10(i_10), m11(i_11), m12(i_12), m13(i_13),
    m20(i_20), m21(i_21), m22(i_22), m23(i_23),
    m30(i_30), m31(i_31), m32(i_32), m33(i_33)
{}

Mat44::Mat44(const Quaternion& i_rotation, const Vec3D& i_translation) :
    m03(i_translation.x()), m13(i_translation.y()), m23(i_translation.z()),
    m30(0.0f), m31(0.0f), m32(0.0f), m33(1.0f)
{
    const auto _2x = i_rotation.x() + i_rotation.x();
    const auto _2y = i_rotation.y() + i_rotation.y();
    const auto _2z = i_rotation.z() + i_rotation.z();
    const auto _2xx = i_rotation.x() * _2x;
    const auto _2xy = _2x * i_rotation.y();
    const auto _2xz = _2x * i_rotation.z();
    const auto _2xw = _2x * i_rotation.w();
    const auto _2yy = _2y * i_rotation.y();
    const auto _2yz = _2y * i_rotation.z();
    const auto _2yw = _2y * i_rotation.w();
    const auto _2zz = _2z * i_rotation.z();
    const auto _2zw = _2z * i_rotation.w();

    m00 = 1.0f - _2yy - _2zz;
    m01 = _2xy - _2zw;
    m02 = _2xz + _2yw;

    m10 = _2xy + _2zw;
    m11 = 1.0f - _2xx - _2zz;
    m12 = _2yz - _2xw;

    m20 = _2xz - _2yw;
    m21 = _2yz + _2xw;
    m22 = 1.0f - _2xx - _2yy;
}

Mat44::~Mat44()
{}

void Mat44::Set(float i_00, float i_01, float i_02, float i_03,
                float i_10, float i_11, float i_12, float i_13,
                float i_20, float i_21, float i_22, float i_23,
                float i_30, float i_31, float i_32, float i_33)
{
    m00 = i_00;
    m01 = i_01;
    m02 = i_02;
    m03 = i_03;

    m10 = i_10;
    m11 = i_11;
    m12 = i_12;
    m13 = i_13;

    m20 = i_20;
    m21 = i_21;
    m22 = i_22;
    m23 = i_23;

    m30 = i_30;
    m31 = i_31;
    m32 = i_32;
    m33 = i_33;
}

Mat44::Mat44(const Mat44& i_copy) :
    m00(i_copy.m00), m01(i_copy.m01), m02(i_copy.m02), m03(i_copy.m03),
    m10(i_copy.m10), m11(i_copy.m11), m12(i_copy.m12), m13(i_copy.m13),
    m20(i_copy.m20), m21(i_copy.m21), m22(i_copy.m22), m23(i_copy.m23),
    m30(i_copy.m30), m31(i_copy.m31), m32(i_copy.m32), m33(i_copy.m33)
{}

Mat44& Mat44::operator=(const Mat44& i_other)
{
    if (this != &i_other)
    {
        m00 = i_other.m00;
        m01 = i_other.m01;
        m02 = i_other.m02;
        m03 = i_other.m03;

        m10 = i_other.m10;
        m11 = i_other.m11;
        m12 = i_other.m12;
        m13 = i_other.m13;

        m20 = i_other.m20;
        m21 = i_other.m21;
        m22 = i_other.m22;
        m23 = i_other.m23;

        m30 = i_other.m30;
        m31 = i_other.m31;
        m32 = i_other.m32;
        m33 = i_other.m33;
    }
    return *this;
}

float Mat44::GetDeterminant() const
{
    float a0 = m00 * m11 - m01 * m10;
    float a1 = m00 * m12 - m02 * m10;
    float a2 = m00 * m13 - m03 * m10;
    float a3 = m01 * m12 - m02 * m11;
    float a4 = m01 * m13 - m03 * m11;
    float a5 = m02 * m13 - m03 * m12;
    float b0 = m20 * m31 - m21 * m30;
    float b1 = m20 * m32 - m22 * m30;
    float b2 = m20 * m33 - m23 * m30;
    float b3 = m21 * m32 - m22 * m31;
    float b4 = m21 * m33 - m23 * m31;
    float b5 = m22 * m33 - m23 * m32;

    return (a0 * b5 + a5 * b0 + a3 * b2 + a2 * b3 - a1 * b4 - a4 * b1);
}

void Mat44::Invert()
{
    float a0 = m00 * m11 - m01 * m10;
    float a1 = m00 * m12 - m02 * m10;
    float a2 = m00 * m13 - m03 * m10;
    float a3 = m01 * m12 - m02 * m11;
    float a4 = m01 * m13 - m03 * m11;
    float a5 = m02 * m13 - m03 * m12;
    float b0 = m20 * m31 - m21 * m30;
    float b1 = m20 * m32 - m22 * m30;
    float b2 = m20 * m33 - m23 * m30;
    float b3 = m21 * m32 - m22 * m31;
    float b4 = m21 * m33 - m23 * m31;
    float b5 = m22 * m33 - m23 * m32;

    float det = a0 * b5 + a5 * b0 + a3 * b2 + a2 * b3 - a1 * b4 - a4 * b1;

    if (FuzzyEquals(det, 0.0f))
    {
        return;
    }

    Mat44 adjoin;
    adjoin.m00 =  m11 * b5 - m12 * b4 + m13 * b3;
    adjoin.m01 = -m01 * b5 + m02 * b4 - m03 * b3;
    adjoin.m02 =  m31 * a5 - m32 * a4 + m33 * a3;
    adjoin.m03 = -m21 * a5 + m22 * a4 - m23 * a3;

    adjoin.m10 = -m10 * b5 + m12 * b2 - m13 * b1;
    adjoin.m11 =  m00 * b5 - m02 * b2 + m03 * b1;
    adjoin.m12 = -m30 * a5 + m32 * a2 - m33 * a1;
    adjoin.m13 =  m20 * a5 - m22 * a2 + m23 * a1;

    adjoin.m20 =  m10 * b4 - m11 * b2 + m13 * b0;
    adjoin.m21 = -m00 * b4 + m01 * b2 - m03 * b0;
    adjoin.m22 =  m30 * a4 - m31 * a2 + m33 * a0;
    adjoin.m23 = -m20 * a4 + m21 * a2 - m23 * a0;

    adjoin.m30 = -m10 * b3 + m11 * b1 - m12 * b0;
    adjoin.m31 =  m00 * b3 - m01 * b1 + m02 * b0;
    adjoin.m32 = -m30 * a3 + m31 * a1 - m32 * a0;
    adjoin.m33 =  m20 * a3 - m21 * a1 + m22 * a0;

    // only divide by determinant if determinant is not 1
    if (!FuzzyEquals(det, 1.0f))
    {
        float inverse_det = 1.0f / det;

        adjoin.m00 *= inverse_det;
        adjoin.m01 *= inverse_det;
        adjoin.m02 *= inverse_det;
        adjoin.m03 *= inverse_det;

        adjoin.m10 *= inverse_det;
        adjoin.m11 *= inverse_det;
        adjoin.m12 *= inverse_det;
        adjoin.m13 *= inverse_det;

        adjoin.m20 *= inverse_det;
        adjoin.m21 *= inverse_det;
        adjoin.m22 *= inverse_det;
        adjoin.m23 *= inverse_det;

        adjoin.m30 *= inverse_det;
        adjoin.m31 *= inverse_det;
        adjoin.m32 *= inverse_det;
        adjoin.m33 *= inverse_det;
    }

    *this = adjoin;
}

Mat44 Mat44::GetInverse() const
{
    Mat44 inverse(*this);
    inverse.Invert();
    return inverse;
}

void Mat44::Transpose()
{
    float temp = m10;
    m10 = m01;
    m01 = temp;

    temp = m20;
    m20 = m02;
    m02 = temp;

    temp = m30;
    m30 = m03;
    m03 = temp;

    temp = m21;
    m21 = m12;
    m12 = temp;

    temp = m31;
    m31 = m13;
    m13 = temp;

    temp = m32;
    m32 = m23;
    m23 = temp;
}

Mat44 Mat44::GetTranspose() const
{
    Mat44 transpose(*this);
    transpose.Transpose();
    return transpose;
}

//Mat44 Mat44::operator*(float i_scalar)
//{
//    // validate input
//    ASSERT(!IsNaN(i_scalar));
//
//    Mat44 result(f11 * i_scalar, f12 * i_scalar, f13 * i_scalar, f14 * i_scalar,
//                 f21 * i_scalar, f22 * i_scalar, f23 * i_scalar, f24 * i_scalar,
//                 f31 * i_scalar, f32 * i_scalar, f33 * i_scalar, f34 * i_scalar,
//                 f41 * i_scalar, f42 * i_scalar, f43 * i_scalar, f44 * i_scalar);
//
//    return result;
//}

bool Mat44::operator==(const Mat44& i_other) const
{
    if (this == &i_other)
    {
        return true;
    }

    return !(!FuzzyEquals(m00, i_other.m00) || !FuzzyEquals(m01, i_other.m01) || !FuzzyEquals(m02, i_other.m02) || !FuzzyEquals(m03, i_other.m03) ||
             !FuzzyEquals(m10, i_other.m10) || !FuzzyEquals(m11, i_other.m11) || !FuzzyEquals(m12, i_other.m12) || !FuzzyEquals(m13, i_other.m13) ||
             !FuzzyEquals(m20, i_other.m20) || !FuzzyEquals(m21, i_other.m21) || !FuzzyEquals(m22, i_other.m22) || !FuzzyEquals(m23, i_other.m23) ||
             !FuzzyEquals(m30, i_other.m30) || !FuzzyEquals(m31, i_other.m31) || !FuzzyEquals(m32, i_other.m32) || !FuzzyEquals(m33, i_other.m33));
}

// this * i_right_mat
Mat44 Mat44::Multiply(const Mat44& i_right_mat) const
{
    Mat44 result;

    result.m00 = m00 * i_right_mat.m00 + m01 * i_right_mat.m10 + m02 * i_right_mat.m20 + m03 * i_right_mat.m30;
    result.m01 = m00 * i_right_mat.m01 + m01 * i_right_mat.m11 + m02 * i_right_mat.m21 + m03 * i_right_mat.m31;
    result.m02 = m00 * i_right_mat.m02 + m01 * i_right_mat.m12 + m02 * i_right_mat.m22 + m03 * i_right_mat.m32;
    result.m03 = m00 * i_right_mat.m03 + m01 * i_right_mat.m13 + m02 * i_right_mat.m23 + m03 * i_right_mat.m33;

    result.m10 = m10 * i_right_mat.m00 + m11 * i_right_mat.m10 + m12 * i_right_mat.m20 + m13 * i_right_mat.m30;
    result.m11 = m10 * i_right_mat.m01 + m11 * i_right_mat.m11 + m12 * i_right_mat.m21 + m13 * i_right_mat.m31;
    result.m12 = m10 * i_right_mat.m02 + m11 * i_right_mat.m12 + m12 * i_right_mat.m22 + m13 * i_right_mat.m32;
    result.m13 = m10 * i_right_mat.m03 + m11 * i_right_mat.m13 + m12 * i_right_mat.m23 + m13 * i_right_mat.m33;

    result.m20 = m20 * i_right_mat.m00 + m21 * i_right_mat.m10 + m22 * i_right_mat.m20 + m23 * i_right_mat.m30;
    result.m21 = m20 * i_right_mat.m01 + m21 * i_right_mat.m11 + m22 * i_right_mat.m21 + m23 * i_right_mat.m31;
    result.m22 = m20 * i_right_mat.m02 + m21 * i_right_mat.m12 + m22 * i_right_mat.m22 + m23 * i_right_mat.m32;
    result.m23 = m20 * i_right_mat.m03 + m21 * i_right_mat.m13 + m22 * i_right_mat.m23 + m23 * i_right_mat.m33;

    result.m30 = m30 * i_right_mat.m00 + m31 * i_right_mat.m10 + m32 * i_right_mat.m20 + m33 * i_right_mat.m30;
    result.m31 = m30 * i_right_mat.m01 + m31 * i_right_mat.m11 + m32 * i_right_mat.m21 + m33 * i_right_mat.m31;
    result.m32 = m30 * i_right_mat.m02 + m31 * i_right_mat.m12 + m32 * i_right_mat.m22 + m33 * i_right_mat.m32;
    result.m33 = m30 * i_right_mat.m03 + m31 * i_right_mat.m13 + m32 * i_right_mat.m23 + m33 * i_right_mat.m33;

    return result;
}

// i_left_vec4 * this
Vec4D Mat44::MultiplyLeft(const Vec4D& i_left_vec4) const
{
    Vec4D result;

    result.x(i_left_vec4.x() * m00 + i_left_vec4.y() * m10 + i_left_vec4.z() * m20 + i_left_vec4.w() * m30);
    result.y(i_left_vec4.x() * m01 + i_left_vec4.y() * m11 + i_left_vec4.z() * m21 + i_left_vec4.w() * m31);
    result.z(i_left_vec4.x() * m02 + i_left_vec4.y() * m12 + i_left_vec4.z() * m22 + i_left_vec4.w() * m32);
    result.w(i_left_vec4.x() * m03 + i_left_vec4.y() * m13 + i_left_vec4.z() * m23 + i_left_vec4.w() * m33);

    return result;
}

// this * i_right_vec4
Vec4D Mat44::MultiplyRight(const Vec4D& i_right_vec4) const
{
    Vec4D result;

    result.x(m00 * i_right_vec4.x() + m01 * i_right_vec4.y() + m02 * i_right_vec4.z() + m03 * i_right_vec4.w());
    result.y(m10 * i_right_vec4.x() + m11 * i_right_vec4.y() + m12 * i_right_vec4.z() + m13 * i_right_vec4.w());
    result.z(m20 * i_right_vec4.x() + m21 * i_right_vec4.y() + m22 * i_right_vec4.z() + m23 * i_right_vec4.w());
    result.w(m30 * i_right_vec4.x() + m31 * i_right_vec4.y() + m32 * i_right_vec4.z() + m33 * i_right_vec4.w());

    return result;
}

#ifdef BUILD_DEBUG
void Mat44::Print() const
{
    LOG("\n%f, \t%f, \t%f, \t%f\n%f, \t%f, \t%f, \t%f\n%f, \t%f, \t%f, \t%f\n%f, \t%f, \t%f, \t%f",
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33);
}
#endif

Mat44 Mat44::GetTranslation(const Vec3D& i_translation)
{
    return Mat44(1.0, 0.0f, 0.0f, i_translation.x(),
                 0.0f, 1.0f, 0.0f, i_translation.y(),
                 0.0f, 0.0f, 1.0f, i_translation.z(),
                 0.0f, 0.0f, 0.0f, 1.0f);
}

Mat44 Mat44::GetRotationX(const float i_radians)
{
    return Mat44(1.0f, 0.0f, 0.0f, 0.0f,
                 0.0f, cosf(i_radians), -sinf(i_radians), 0.0f,
                 0.0f, sinf(i_radians), cosf(i_radians), 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
}

Mat44 Mat44::GetRotationY(const float i_radians)
{
    return Mat44(cosf(i_radians), 0, sinf(i_radians), 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 -sinf(i_radians), 0.0f, cosf(i_radians), 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
}

Mat44 Mat44::GetRotationZ(const float i_radians)
{
    return Mat44(cosf(i_radians), -sinf(i_radians), 0.0f, 0.0f,
                 sinf(i_radians), cosf(i_radians), 0.0f, 0.0f,
                 0.0f, 0.0f, 1.0f, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
}

Mat44 Mat44::GetScale(const float i_scale)
{
    return Mat44(i_scale, 0.0f, 0.0f, 0.0f,
                 0.0f, i_scale, 0.0f, 0.0f,
                 0.0f, 0.0f, i_scale, 0.0f,
                 0.0f, 0.0f, 0.0f, 1.0f);
}

} // namespace math
} // namespace engine