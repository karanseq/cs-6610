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

enum class ESkeletonType : uint8_t
{
    None,
    SimpleChain,
    Humanoid,
    Palm,

}; // enum class ESkeletonType

struct Skeleton;
struct Joint
{
    engine::math::Transform     local_to_parent;
    uint8_t                     parent_index = MAX_JOINT_INDEX;
    uint8_t                     child_index = MAX_JOINT_INDEX;

    static constexpr uint8_t MAX_JOINT_INDEX = 255;

}; // struct Joint

struct Skeleton
{
    ESkeletonType               type = ESkeletonType::None;
    Joint*                      joints = nullptr;
    cy::Matrix4f*               local_to_world_transforms = nullptr;
    cy::Matrix4f*               world_to_local_transforms = nullptr;
    engine::math::Transform*    cached_pose = nullptr;
    engine::math::Vec3D*        joints_world_space = nullptr;
    float                       bone_length = 0.0f;
    uint8_t                     num_joints = 0;

    void UpdateJointTransforms(uint8_t i_index);
    void UpdateChain(uint8_t i_start_index, uint8_t i_end_index);
    void UpdateChain();
    void UpdateJointWorldSpacePositions();
    void ResetToCachedPose();

    inline bool IsEndEffector(uint8_t i_index) const
    {
        switch (type)
        {
        case engine::animation::ESkeletonType::SimpleChain:
            return i_index == num_joints - 1;
        case engine::animation::ESkeletonType::Humanoid:
            return i_index == LEFT_FOOT || i_index == RIGHT_FOOT || i_index == LEFT_HAND || i_index == RIGHT_HAND;
        case engine::animation::ESkeletonType::None:
        case engine::animation::ESkeletonType::Palm:
        default:
            return false;
        }
    }

    static void CreateSkeleton(Skeleton*& io_skeleton, ESkeletonType i_type);

    static void InitSimpleChain(Skeleton*& io_skeleton);
    static void InitHumanoid(Skeleton*& io_skeleton);
    static void InitPalm(Skeleton*& io_skeleton);

    static constexpr uint8_t PELVIS = 0;
    static constexpr uint8_t TORSO = 1;
    static constexpr uint8_t UP_LEFT_LEG = 2;
    static constexpr uint8_t UP_RIGHT_LEG = 3;
    static constexpr uint8_t UP_LEFT_ARM = 4;
    static constexpr uint8_t UP_RIGHT_ARM = 5;
    static constexpr uint8_t HEAD = 6;
    static constexpr uint8_t LOW_LEFT_LEG = 7;
    static constexpr uint8_t LOW_RIGHT_LEG = 8;
    static constexpr uint8_t LOW_LEFT_ARM = 9;
    static constexpr uint8_t LOW_RIGHT_ARM = 10;
    static constexpr uint8_t LEFT_FOOT = 11;
    static constexpr uint8_t RIGHT_FOOT = 12;
    static constexpr uint8_t LEFT_HAND = 13;
    static constexpr uint8_t RIGHT_HAND = 14;

}; // struct Skeleton

} // namespace animation
} // namespace engine

#endif // ENGINE_SKELETON_H_