#ifndef ENGINE_SKELETON_H_
#define ENGINE_SKELETON_H_

// Library includes
#include <stdint.h>

// Engine includes
#include "Math/Transform.h"

// Util includes
#include "Utils/cyMatrix.h"

namespace engine {

// Forward declarations
namespace math {
    class Quaternion;
}

namespace animation {

enum class ESkeletonType : uint8_t {

    SimpleChain,
    Humanoid,
    Palm,

}; // enum class ESkeletonType

struct Joint
{
    engine::math::Transform     local_to_parent;
    uint8_t                     parent_index = 0;
    uint8_t                     child_index = 0;

}; // struct Joint

struct Skeleton
{
    Joint*                      joints = nullptr;
    cy::Matrix4f*               local_to_world_transforms = nullptr;
    cy::Matrix4f*               world_to_local_transforms = nullptr;
    engine::math::Vec3D*        solved_joints = nullptr;
    engine::math::Quaternion*   local_to_world_rotations = nullptr;
    engine::math::Quaternion*   world_to_local_rotations = nullptr;
    float                       bone_length = 0.0f;
    uint16_t                    num_joints = 0;

    static void CreateSkeleton(Skeleton*& io_skeleton, ESkeletonType i_type);

    static void InitSimpleChain(Skeleton*& io_skeleton);
    static void InitHumanoid(Skeleton*& io_skeleton);
    static void InitPalm(Skeleton*& io_skeleton);

}; // struct Skeleton

} // namespace animation
} // namespace engine

#endif // ENGINE_SKELETON_H_