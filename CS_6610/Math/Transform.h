#ifndef ENGINE_TRANSFORM_H_
#define ENGINE_TRANSFORM_H_

#include "Quaternion.h"
#include "Vec3D.h"

namespace engine {
namespace math {

    /*
        Transform
        - A class that represents the position, scale and rotation of an object in 3D space
    */

    struct Transform
    {
    public:
        explicit Transform(const Quaternion& i_rotation = Quaternion::IDENTITY,
            const Vec3D& i_position = Vec3D::ZERO,
            float i_scale = 1.0f);
        Transform(const Transform& i_copy);

        ~Transform() = default;

        // assignment
        inline Transform& operator=(const Transform& i_transform);

        // constants
        static const Transform          ZERO;

        Quaternion                      rotation_;
        Vec3D                           position_;
        float                           scale_;

    }; // struct Transform

} // namespace math
} // namespace engine

#include "Transform-inl.h"

#endif // ENGINE_TRANSFORM_H_
