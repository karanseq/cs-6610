#include "Transform.h"

namespace engine {
namespace math {

    const Transform Transform::ZERO;

    Transform::Transform(const Quaternion& i_rotation/* = Quaternion::IDENTITY*/,
        const Vec3D& i_position/* = Vec3D::ZERO*/,
        float i_scale/* = 1.0f*/) : 
        rotation_(i_rotation),
        position_(i_position),
        scale_(i_scale)
    {}

    Transform::Transform(const Transform& i_copy) : 
        rotation_(i_copy.rotation_),
        position_(i_copy.position_),
        scale_(i_copy.scale_)
    {}

} // namespace math
} // namespace engine
