#include "FABRIK.h"

// Engine includes
#include "Animation/Skeleton.h"

// Util includes
#include "Utils/logger.h"

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
        SolveForward(i_params);
        SolveBackward(i_params);
    }
}

void SolveForward(const FABRIKParams& i_params)
{
    uint16_t joint_index = i_params.end_joint_index;
    uint16_t parent_index = i_params.skeleton->joints[i_params.end_joint_index].parent_index;
    
    i_params.solved_joints[joint_index] = i_params.target;

    // Keep going till we reach the root of the chain
    while (joint_index != i_params.root_joint_index)
    {
        const cy::Point3f parent_trans = i_params.skeleton->joint_to_world_transforms[parent_index].GetTrans();
        const engine::math::Vec3D parent_world_space(parent_trans.x, parent_trans.y, parent_trans.z);
        const engine::math::Vec3D joint_to_parent = parent_world_space - i_params.solved_joints[joint_index];
        const engine::math::Vec3D joint_to_parent_normalized = joint_to_parent.Normalize();

        i_params.solved_joints[parent_index] = i_params.solved_joints[joint_index] + (joint_to_parent_normalized * i_params.skeleton->bone_length);

        // Update joint indices
        joint_index = parent_index;
        parent_index = i_params.skeleton->joints[joint_index].parent_index;
    }

    UpdateRotations(i_params);
}

void SolveBackward(const FABRIKParams& i_params)
{
    uint16_t joint_index = i_params.root_joint_index;
    uint16_t child_index = joint_index + 1;

    const cy::Point3f joint_trans = i_params.skeleton->joint_to_world_transforms[joint_index].GetTrans();
    i_params.solved_joints[joint_index].set(joint_trans.x, joint_trans.y, joint_trans.z);

    // Keep going till we reach the end
    while (child_index != i_params.end_joint_index)
    {
        const cy::Point3f child_trans = i_params.skeleton->joint_to_world_transforms[child_index].GetTrans();
        const engine::math::Vec3D child_world_space(child_trans.x, child_trans.y, child_trans.z);
        const engine::math::Vec3D joint_to_child = child_world_space - i_params.solved_joints[joint_index];
        const engine::math::Vec3D joint_to_child_normalized = joint_to_child.Normalize();

        i_params.solved_joints[child_index] = i_params.solved_joints[joint_index] + (joint_to_child_normalized * i_params.skeleton->bone_length);

        // Update joint indices
        joint_index = child_index;
        child_index = joint_index + 1;
    }

    UpdateRotations(i_params);
}

void UpdateRotations(const FABRIKParams& i_params)
{
    uint16_t joint_index = i_params.end_joint_index;
    uint16_t parent_index = i_params.skeleton->joints[i_params.end_joint_index].parent_index;

    while (joint_index != i_params.root_joint_index)
    {
        // Calculate parent to joint direction before solving
        const cy::Point3f joint_trans = i_params.skeleton->joint_to_world_transforms[joint_index].GetTrans();
        const engine::math::Vec3D joint_world_space(joint_trans.x, joint_trans.y, joint_trans.z);
        const cy::Point3f parent_trans = i_params.skeleton->joint_to_world_transforms[parent_index].GetTrans();
        const engine::math::Vec3D parent_world_space(parent_trans.x, parent_trans.y, parent_trans.z);
        engine::math::Vec3D parent_to_joint_before_solving = joint_world_space - parent_world_space;
        parent_to_joint_before_solving.Normalize();

        // Calculate parent to joint direction after solving
        engine::math::Vec3D parent_to_joint_after_solving = i_params.solved_joints[joint_index] - i_params.solved_joints[parent_index];
        parent_to_joint_after_solving.Normalize();

        // Calculate rotation from before to after
        engine::math::Quaternion rotation = engine::math::Quaternion::GetShortestRotation(parent_to_joint_before_solving, parent_to_joint_after_solving);
        i_params.skeleton->joints[parent_index].local_to_parent.rotation_ = rotation * i_params.skeleton->joints[parent_index].local_to_parent.rotation_;

        // Update joint indices
        joint_index = parent_index;
        parent_index = i_params.skeleton->joints[joint_index].parent_index;
    }
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
