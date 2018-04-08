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

    const float chain_length = CalculateChainLength(i_params.skeleton, i_params.root_joint_index, i_params.end_joint_index);

    const cy::Point3f root_world_space_trans = i_params.skeleton->local_to_world_transforms[i_params.root_joint_index].GetTrans();
    const engine::math::Vec3D root_world_space(root_world_space_trans.x, root_world_space_trans.y, root_world_space_trans.z);
    const engine::math::Vec3D root_to_target_world_space = i_params.target - engine::math::Vec3D(root_world_space_trans.x, root_world_space_trans.y, root_world_space_trans.z);

    // Is the target within reach?
    if (root_to_target_world_space.LengthSquared() > chain_length * chain_length)
    {
        // When the target is out of reach,
        // we simply stretch the chain from root to target
        //for (uint8_t i = 0; i < i_params.iterations / 2; ++i)
        //{
        //    for (uint16_t i = i_params.root_joint_index + 1; i <= i_params.end_joint_index; ++i)
        //    {
        //        const engine::math::Vec3D root_to_target_joint_space = i_params.skeleton->world_to_local_transforms[i] * root_to_target_world_space;
        //        const engine::math::Vec3D root_to_target_joint_space_normalized = root_to_target_joint_space.Normalize();

        //        i_params.skeleton->joints[i].local_to_parent.position_ = root_to_target_joint_space_normalized * i_params.skeleton->bone_length;
        //    }
        //}

        const engine::math::Vec3D root_to_target_world_space_normalized = root_to_target_world_space.Normalize();
        uint8_t joint_index = i_params.root_joint_index;
        while (joint_index != i_params.end_joint_index)
        {
            const uint8_t child_index = i_params.skeleton->joints[joint_index].child_index;
            i_params.skeleton->joints_world_space[child_index] = i_params.skeleton->joints_world_space[joint_index] + root_to_target_world_space_normalized * i_params.skeleton->bone_length;

            joint_index = child_index;
        }
    }
    else
    {
        // When the target is within reach,
        // we solve till either the end effector is "close enough"
        // or we run out of iterations
        for (uint8_t i = 0; i < i_params.iterations; ++i)
        {
            if ((i_params.skeleton->joints_world_space[i_params.end_joint_index] - i_params.target).LengthSquared() < i_params.tolerance)
            {
                break;
            }

            LOG("Iteration-%d", i);
            
            SolveForward(i_params);
            LOG("After SolveForward");
            PrintSolvedJoints(i_params);

            SolveBackward(i_params);
            LOG("After SolveBackward");
            PrintSolvedJoints(i_params);

            LOG("----------------------------------------------------------------------");
        }
    }

    UpdateRotations(i_params);
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
#if 0

    uint8_t joint_index = i_params.end_joint_index;
    uint8_t parent_index = i_params.skeleton->joints[i_params.end_joint_index].parent_index;

    while (joint_index != i_params.root_joint_index)
    {
        // Calculate parent to joint direction before solving
        const cy::Point3f joint_trans = i_params.skeleton->local_to_world_transforms[joint_index].GetTrans();
        const engine::math::Vec3D joint_world_space(joint_trans.x, joint_trans.y, joint_trans.z);
        const cy::Point3f parent_trans = i_params.skeleton->local_to_world_transforms[parent_index].GetTrans();
        const engine::math::Vec3D parent_world_space(parent_trans.x, parent_trans.y, parent_trans.z);
        engine::math::Vec3D parent_to_joint_before_solving = joint_world_space - parent_world_space;
        parent_to_joint_before_solving.Normalize();

        // Calculate parent to joint direction after solving
        engine::math::Vec3D parent_to_joint_after_solving = i_params.skeleton->solved_joints[joint_index] - i_params.skeleton->solved_joints[parent_index];
        parent_to_joint_after_solving.Normalize();

        // Calculate rotation from before to after
        engine::math::Quaternion rotation_world_space = engine::math::Quaternion::GetShortestRotation(parent_to_joint_before_solving, parent_to_joint_after_solving);
        //i_params.skeleton->joints[parent_index].local_to_parent.rotation_ = rotation_world_space * i_params.skeleton->joints[parent_index].local_to_parent.rotation_;

        i_params.skeleton->joints[parent_index].local_to_parent.rotation_ = i_params.skeleton->world_to_local_rotations[parent_index] * rotation_world_space;

        // Update joint indices
        joint_index = parent_index;
        parent_index = i_params.skeleton->joints[joint_index].parent_index;
    }

#else

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
        //i_params.skeleton->joints[parent_index].local_to_parent.rotation_ = rotation_parent_space * rotation_parent_space * i_params.skeleton->joints[parent_index].local_to_parent.rotation_;
        
        // Update matrices
        //i_params.skeleton->UpdateJointTransform(parent_index);
        //i_params.skeleton->UpdateJointTransform(child_index);
        i_params.skeleton->UpdateChain(parent_index, i_params.end_joint_index);

        // Update joint indices
        parent_index = child_index;
        child_index = i_params.skeleton->joints[parent_index].child_index;
    }

#endif
}

float CalculateChainLength(const Skeleton* i_skeleton, uint8_t i_rootJointIndex, uint8_t i_endJointIndex)
{
    if (i_rootJointIndex >= i_skeleton->num_joints ||
        i_endJointIndex >= i_skeleton->num_joints ||
        i_rootJointIndex == i_endJointIndex)
    {
        return -1.0f;
    }

    float chain_length = i_skeleton->bone_length;

    uint8_t parent_joint_index = i_skeleton->joints[i_endJointIndex].parent_index;
    uint8_t counter = i_skeleton->num_joints;
    while (parent_joint_index != i_rootJointIndex &&
        counter > 0)
    {
        chain_length += i_skeleton->bone_length;
        parent_joint_index = i_skeleton->joints[parent_joint_index].parent_index;
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
