#ifndef ENGINE_SKELETON_H_
#define ENGINE_SKELETON_H_

// Library includes
#include <stdint.h>

// Engine includes
#include "Math/Transform.h"

// Util includes
#include "Utils/cyMatrix.h"

namespace engine {
namespace animation {

struct Joint
{
    engine::math::Transform     local_to_parent;
    uint8_t                     parent_index = 0;

}; // struct Joint

struct Skeleton
{
    Joint*                      joints = nullptr;
    cy::Matrix4f*               global_joint_transforms = nullptr;
    float                       bone_length = 0.0f;
    uint16_t                    num_joints = 0;

}; // struct Skeleton



} // namespace animation
} // namespace engine

#endif // ENGINE_SKELETON_H_