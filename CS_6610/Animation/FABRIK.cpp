#include "FABRIK.h"

// Engine includes
#include "Animation/Skeleton.h"

namespace engine {
namespace animation {

void FABRIK(const FABRIKParams& i_params)
{
    const float chain_length = CalculateChainLength(i_params.skeleton, i_params.root_joint_index, i_params.end_joint_index);

    const cy::Point3f root_trans = i_params.skeleton->joint_to_world_transforms[i_params.root_joint_index].GetTrans();
    const engine::math::Vec3D root_world_space(root_trans.x, root_trans.y, root_trans.z);
    const engine::math::Vec3D root_to_target = i_params.target - engine::math::Vec3D(root_trans.x, root_trans.y, root_trans.z);

    // Is the target within reach?
    if (root_to_target.LengthSquared() > chain_length * chain_length)
    {
        // When the target is out of reach,
        // we simply stretch the chain from root to target
        const cy::Point3f root_trans = i_params.skeleton->joint_to_world_transforms[i_params.root_joint_index].GetTrans();        
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
        SolveBackward(i_params);
        SolveForward(i_params);
    }
}

void SolveForward(const FABRIKParams& i_params)
{

}

void SolveBackward(const FABRIKParams& i_params)
{

}

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
