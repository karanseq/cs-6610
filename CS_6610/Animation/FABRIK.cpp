#include "FABRIK.h"

// Engine includes
#include "Animation/Skeleton.h"

namespace engine {
namespace animation {

void FABRIK(const FABRIKParams& i_params)
{
    const float chain_length = CalculateChainLength(i_params.skeleton, i_params.root_joint_index, i_params.end_joint_index);

    const engine::math::Vec3D end_world_space = i_params.skeleton->joint_to_world_transforms[i_params.end_joint_index] * engine::math::Vec3D::ZERO;
    const float end_to_target_length_squared = (i_params.target - end_world_space).LengthSquared();

    // Is the target within reach?
    if (end_to_target_length_squared > chain_length * chain_length)
    {
        // When the target is out of reach,
        // we simply stretch the chain from root to target
        const engine::math::Vec3D root_to_target = i_params.target - (i_params.skeleton->joint_to_world_transforms[i_params.root_joint_index] * i_params.skeleton->joints[i_params.root_joint_index].local_to_parent.position_);
        const engine::math::Vec3D root_to_target_normalized = root_to_target.Normalize();

        for (uint16_t i = i_params.root_joint_index + 1; i <= i_params.end_joint_index; ++i)
        {
            i_params.skeleton->joints[i].local_to_parent.position_ = root_to_target_normalized * i_params.skeleton->bone_length;
        }
    }
    else
    {
        // When the target is within reach,
        // we solve till either the end effector is "close enough"
        // or we run out of iterations

    }
}

void SolveForward(Skeleton* io_skeleton, const engine::math::Vec3D& i_target)
{}

void SolveBackward(Skeleton* io_skeleton, const engine::math::Vec3D& i_target)
{}

float CalculateChainLength(const Skeleton* i_skeleton, uint16_t i_rootJointIndex, uint16_t i_endJointIndex)
{
    if (i_rootJointIndex >= i_skeleton->num_joints ||
        i_endJointIndex >= i_skeleton->num_joints ||
        i_rootJointIndex == i_endJointIndex)
    {
        return -1.0f;
    }

    float chain_length = i_skeleton->bone_length;

    uint16_t parent_joint_index = i_skeleton->joints[i_endJointIndex].parent_index;
    uint16_t counter = i_skeleton->num_joints;
    while (parent_joint_index != i_rootJointIndex &&
        counter > 0)
    {
        chain_length += i_skeleton->bone_length;
        parent_joint_index = i_skeleton->joints[parent_joint_index].parent_index;
        --counter;
    }

    return chain_length;
}

} // namespace animation
} // namespace engine
