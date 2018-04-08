#include "FABRIK.h"

// Engine includes
#include "Animation/Skeleton.h"

// Util includes
#include "Utils/logger.h"

namespace engine {
namespace animation {

void FABRIK(const FABRIKParams& i_params)
{
    if (i_params.skeleton == nullptr ||
        i_params.root_joint_index == i_params.end_joint_index ||
        i_params.iterations == 0)
    {
        return;
    }

    bool mustUpdateRotations = false;
    const engine::math::Vec3D root_to_target_world_space = i_params.target - i_params.skeleton->joints_world_space[i_params.root_joint_index];
    const float chain_length = CalculateChainLength(i_params);

    // Is the target within reach?
    if (root_to_target_world_space.LengthSquared() > chain_length * chain_length)
    {
        // When the target is out of reach,
        // we simply stretch the chain from root to target
        const engine::math::Vec3D root_to_target_world_space_normalized = root_to_target_world_space.Normalize();
        uint8_t joint_index = i_params.root_joint_index;
        while (joint_index != i_params.end_joint_index)
        {
            const uint8_t child_index = i_params.skeleton->joints[joint_index].child_index;
            i_params.skeleton->joints_world_space[child_index] = i_params.skeleton->joints_world_space[joint_index] + root_to_target_world_space_normalized * i_params.skeleton->bone_length;

            joint_index = child_index;
        }

        mustUpdateRotations = true;
    }
    else
    {
        // When the target is within reach,
        // we solve till either the end effector is "close enough"
        // or we run out of iterations
        uint8_t i = 0;
        for (i = 0; i < i_params.iterations; ++i)
        {
            if ((i_params.skeleton->joints_world_space[i_params.end_joint_index] - i_params.target).LengthSquared() < i_params.tolerance)
            {
                break;
            }

            SolveForward(i_params);
            SolveBackward(i_params);
            mustUpdateRotations = true;
        }

#if 0
        LOG("FABRIK solved in %d iterations!", i);
#endif
    }

    if (mustUpdateRotations)
    {
        UpdateRotations(i_params);
    }
}

void SolveForward(const FABRIKParams& i_params)
{
    uint8_t joint_index = i_params.end_joint_index;
    uint8_t parent_index = i_params.skeleton->joints[i_params.end_joint_index].parent_index;
    
    i_params.skeleton->joints_world_space[joint_index] = i_params.target;

    // Keep going till we reach the root of the chain
    while (joint_index != i_params.root_joint_index)
    {
        const engine::math::Vec3D joint_to_parent = i_params.skeleton->joints_world_space[parent_index] - i_params.skeleton->joints_world_space[joint_index];
        const engine::math::Vec3D joint_to_parent_normalized = joint_to_parent.Normalize();

        i_params.skeleton->joints_world_space[parent_index] = i_params.skeleton->joints_world_space[joint_index] + (joint_to_parent_normalized * i_params.skeleton->bone_length);

        // Update joint indices
        joint_index = parent_index;
        parent_index = i_params.skeleton->joints[joint_index].parent_index;
    }
}

void SolveBackward(const FABRIKParams& i_params)
{
    uint8_t joint_index = i_params.root_joint_index;
    uint8_t child_index = i_params.skeleton->joints[joint_index].child_index;

    const cy::Point3f joint_trans = i_params.skeleton->local_to_world_transforms[joint_index].GetTrans();
    i_params.skeleton->joints_world_space[joint_index].set(joint_trans.x, joint_trans.y, joint_trans.z);

    // Keep going till we reach the end
    while (child_index != i_params.end_joint_index)
    {
        const engine::math::Vec3D joint_to_child = i_params.skeleton->joints_world_space[child_index] - i_params.skeleton->joints_world_space[joint_index];
        const engine::math::Vec3D joint_to_child_normalized = joint_to_child.Normalize();

        i_params.skeleton->joints_world_space[child_index] = i_params.skeleton->joints_world_space[joint_index] + (joint_to_child_normalized * i_params.skeleton->bone_length);

        // Update joint indices
        joint_index = child_index;
        child_index = i_params.skeleton->joints[joint_index].child_index;
    }
}

void UpdateRotations(const FABRIKParams& i_params)
{
    uint8_t parent_index = i_params.root_joint_index;
    uint8_t child_index = i_params.skeleton->joints[parent_index].child_index;

    while (parent_index != i_params.end_joint_index)
    {
        // Calculate parent to child direction before solving
        const cy::Point4f child_trans_parent_space = i_params.skeleton->world_to_local_transforms[parent_index] * i_params.skeleton->local_to_world_transforms[child_index].GetTrans();
        engine::math::Vec3D direction_before_solving_parent_space(child_trans_parent_space.x, child_trans_parent_space.y, child_trans_parent_space.z);
        direction_before_solving_parent_space.Normalize();

        // Calculate parent to child direction after solving
        engine::math::Vec3D direction_after_solving_parent_space = i_params.skeleton->world_to_local_transforms[parent_index] * i_params.skeleton->joints_world_space[child_index];
        direction_after_solving_parent_space.Normalize();

        // Calculate rotation from before to after
        engine::math::Quaternion rotation_parent_space = engine::math::Quaternion::GetShortestRotation(direction_before_solving_parent_space, direction_after_solving_parent_space);

        // Update rotation
        i_params.skeleton->joints[parent_index].local_to_parent.rotation_ = rotation_parent_space * i_params.skeleton->joints[parent_index].local_to_parent.rotation_;
        i_params.skeleton->joints[parent_index].local_to_parent.rotation_.Normalize();
        
        // Update matrices
        i_params.skeleton->UpdateChain(parent_index, i_params.end_joint_index);

        // Update joint indices
        parent_index = child_index;
        child_index = i_params.skeleton->joints[parent_index].child_index;
    }
}

float CalculateChainLength(const FABRIKParams& i_params)
{
    if (i_params.root_joint_index >= i_params.skeleton->num_joints ||
        i_params.end_joint_index >= i_params.skeleton->num_joints ||
        i_params.root_joint_index == i_params.end_joint_index)
    {
        return -1.0f;
    }

    float chain_length = i_params.skeleton->bone_length;

    uint8_t parent_joint_index = i_params.skeleton->joints[i_params.end_joint_index].parent_index;
    uint8_t counter = i_params.skeleton->num_joints;
    while (parent_joint_index != i_params.root_joint_index &&
        counter > 0)
    {
        chain_length += i_params.skeleton->bone_length;
        parent_joint_index = i_params.skeleton->joints[parent_joint_index].parent_index;
        --counter;
    }

    return chain_length;
}

void PrintSolvedJoints(const FABRIKParams& i_params)
{
    for (uint8_t i = 0; i < i_params.skeleton->num_joints; ++i)
    {
        const engine::math::Vec3D& position = i_params.skeleton->joints_world_space[i];
        LOG("Joint-%d  Position:%f, %f, %f", i, position.x_, position.y_, position.z_);
    }
}

} // namespace animation
} // namespace engine
