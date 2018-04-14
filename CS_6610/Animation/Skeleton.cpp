#include "Skeleton.h"

// Util includes
#include "Utils/cyMatrix.h"
#include "Utils/cyPoint.h"

namespace engine {
namespace animation {

void Skeleton::UpdateJointTransforms(uint8_t i_index)
{
    if (i_index >= num_joints)
    {
        return;
    }

    if (i_index == 0)
    {
        cy::Matrix4f::GetMatrixFromTransform(local_to_world_transforms[0], joints[0].local_to_parent);
        world_to_local_transforms[0] = local_to_world_transforms[0].GetInverse();
    }
    else if (i_index < num_joints)
    {
        // Get the parent's index
        const uint8_t& parent_index = joints[i_index].parent_index;

        // Update matrices
        {
            // Get local to parent transformation matrix
            cy::Matrix4f local_to_parent_transform;
            cy::Matrix4f::GetMatrixFromTransform(local_to_parent_transform, joints[i_index].local_to_parent);

            // Get parent to world transformation matrix
            cy::Matrix4f& parent_to_world_transform = local_to_world_transforms[parent_index];

            // Set local to world & world to local transformation matrix
            local_to_world_transforms[i_index] = parent_to_world_transform * local_to_parent_transform;
            world_to_local_transforms[i_index] = local_to_world_transforms[i_index].GetInverse();
        }
    }
}

void Skeleton::UpdateChain(uint8_t i_start_index, uint8_t i_end_index)
{
    if (i_start_index >= num_joints ||
        i_end_index >= num_joints)
    {
        return;
    }

    UpdateJointTransforms(i_start_index);
    while (i_start_index != i_end_index)
    {
        i_start_index = joints[i_start_index].child_index;
        UpdateJointTransforms(i_start_index);
    }
}

void Skeleton::UpdateChain()
{
    switch (type)
    {
    case engine::animation::ESkeletonType::SimpleChain:
        UpdateChain(0, num_joints - 1);
        break;
    case engine::animation::ESkeletonType::Humanoid:
        UpdateJointTransforms(PELVIS);
        UpdateJointTransforms(TORSO);
        UpdateJointTransforms(HEAD);
        UpdateChain(UP_LEFT_LEG, LEFT_FOOT);
        UpdateChain(UP_RIGHT_LEG, RIGHT_FOOT);
        UpdateChain(UP_LEFT_ARM, LEFT_HAND);
        UpdateChain(UP_RIGHT_ARM, RIGHT_HAND);
        break;
    }
}

void Skeleton::UpdateJointWorldSpacePositions()
{
    for (uint8_t i = 0; i < num_joints; ++i)
    {
        const cy::Point3f joint_trans_world_space = local_to_world_transforms[i].GetTrans();
        joints_world_space[i].set(joint_trans_world_space.x, joint_trans_world_space.y, joint_trans_world_space.z);
    }
}

void Skeleton::ResetToCachedPose()
{
    for (uint8_t i = 0; i < num_joints; ++i)
    {
        joints[i].local_to_parent = cached_pose[i];
    }
}

void Skeleton::CreateSkeleton(Skeleton*& io_skeleton, ESkeletonType i_type)
{
    if (i_type == ESkeletonType::None)
    {
        return;
    }

    io_skeleton = new Skeleton;
    io_skeleton->type = i_type;

    switch (i_type)
    {
    case engine::animation::ESkeletonType::SimpleChain:
        io_skeleton->num_joints = 10;
        io_skeleton->bone_length = 3.0f;
        break;
    case engine::animation::ESkeletonType::Humanoid:
        io_skeleton->num_joints = 15;
        io_skeleton->bone_length = 5.0f;
        break;
    case engine::animation::ESkeletonType::Palm:
        return;
    default:
        break;
    }

    // Allocate joints
    io_skeleton->joints = new engine::animation::Joint[io_skeleton->num_joints];
    io_skeleton->local_to_world_transforms = new cy::Matrix4f[io_skeleton->num_joints];
    io_skeleton->world_to_local_transforms = new cy::Matrix4f[io_skeleton->num_joints];
    io_skeleton->cached_pose = new engine::math::Transform[io_skeleton->num_joints];
    io_skeleton->joints_world_space = new engine::math::Vec3D[io_skeleton->num_joints];

    switch (i_type)
    {
    case engine::animation::ESkeletonType::SimpleChain:
        InitSimpleChain(io_skeleton);
        break;
    case engine::animation::ESkeletonType::Humanoid:
        InitHumanoid(io_skeleton);
        break;
    case engine::animation::ESkeletonType::Palm:
        InitPalm(io_skeleton);
        break;
    default:
        break;
    }

    io_skeleton->UpdateChain();
}

void Skeleton::InitSimpleChain(Skeleton*& io_skeleton)
{
    // Initialize the parent indices
    io_skeleton->joints[0].parent_index = 0;
    for (uint8_t i = 1; i < io_skeleton->num_joints; ++i)
    {
        const uint8_t parent_index = i - 1;
        io_skeleton->joints[i].parent_index = parent_index;
        io_skeleton->joints[parent_index].child_index = i;
    }

    // Root
    io_skeleton->joints[0].local_to_parent.rotation_ = engine::math::Quaternion(0.0f, engine::math::Vec3D::UNIT_Z);
    io_skeleton->joints[0].local_to_parent.position_ = engine::math::Vec3D::ZERO;
    io_skeleton->cached_pose[0] = io_skeleton->joints[0].local_to_parent;

    // Chain
    for (uint8_t i = 1; i < io_skeleton->num_joints; ++i)
    {
        io_skeleton->joints[i].local_to_parent.rotation_ = engine::math::Quaternion(0.0f, engine::math::Vec3D::UNIT_Z);
        io_skeleton->joints[i].local_to_parent.position_.set(0.0f, io_skeleton->bone_length, 0.0f);
        io_skeleton->cached_pose[i] = io_skeleton->joints[i].local_to_parent;
    }
}

void Skeleton::InitHumanoid(Skeleton*& io_skeleton)
{
    // Reset the transforms
    for (uint8_t i = 0; i < io_skeleton->num_joints; ++i)
    {
        io_skeleton->joints[i].local_to_parent.position_ = engine::math::Vec3D::ZERO;
        io_skeleton->joints[i].local_to_parent.rotation_ = engine::math::Quaternion::FORWARD * engine::math::Quaternion::FORWARD;
    }

    //-----------------------
    // The Skeletal Structure
    //
    //            6
    //            |
    // 14--10--5--1--4--9--13
    //            |
    //            0
    //          /   \
    //         3     2
    //         |     |
    //         8     7
    //         |     |
    //         12    11
    //
    //-----------------------

    // Root
    io_skeleton->joints[PELVIS].local_to_parent.position_.y_ = 3 * io_skeleton->bone_length;

    // Spine
    io_skeleton->joints[TORSO].parent_index = PELVIS;
    io_skeleton->joints[TORSO].local_to_parent.position_.y_ = io_skeleton->bone_length;
    io_skeleton->joints[HEAD].parent_index = TORSO;
    io_skeleton->joints[HEAD].local_to_parent.position_.y_ = io_skeleton->bone_length;

    // Left leg
    io_skeleton->joints[UP_LEFT_LEG].parent_index = PELVIS;
    io_skeleton->joints[UP_LEFT_LEG].child_index = LOW_LEFT_LEG;
    io_skeleton->joints[UP_LEFT_LEG].local_to_parent.position_.x_ = 0.75f * io_skeleton->bone_length;
    io_skeleton->joints[UP_LEFT_LEG].local_to_parent.position_.y_ = -io_skeleton->bone_length;
    io_skeleton->joints[LOW_LEFT_LEG].parent_index = UP_LEFT_LEG;
    io_skeleton->joints[LOW_LEFT_LEG].child_index = LEFT_FOOT;
    io_skeleton->joints[LOW_LEFT_LEG].local_to_parent.position_.y_ = -io_skeleton->bone_length;
    io_skeleton->joints[LEFT_FOOT].parent_index = LOW_LEFT_LEG;
    io_skeleton->joints[LEFT_FOOT].local_to_parent.position_.y_ = -io_skeleton->bone_length;

    // Right leg
    io_skeleton->joints[UP_RIGHT_LEG].parent_index = PELVIS;
    io_skeleton->joints[UP_RIGHT_LEG].child_index = LOW_RIGHT_LEG;
    io_skeleton->joints[UP_RIGHT_LEG].local_to_parent.position_.x_ = -0.75f * io_skeleton->bone_length;
    io_skeleton->joints[UP_RIGHT_LEG].local_to_parent.position_.y_ = -io_skeleton->bone_length;
    io_skeleton->joints[LOW_RIGHT_LEG].parent_index = UP_RIGHT_LEG;
    io_skeleton->joints[LOW_RIGHT_LEG].child_index = RIGHT_FOOT;
    io_skeleton->joints[LOW_RIGHT_LEG].local_to_parent.position_.y_ = -io_skeleton->bone_length;
    io_skeleton->joints[RIGHT_FOOT].parent_index = LOW_RIGHT_LEG;
    io_skeleton->joints[RIGHT_FOOT].local_to_parent.position_.y_ = -io_skeleton->bone_length;

    // Left arm
    io_skeleton->joints[UP_LEFT_ARM].parent_index = TORSO;
    io_skeleton->joints[UP_LEFT_ARM].child_index = LOW_LEFT_ARM;
    io_skeleton->joints[UP_LEFT_ARM].local_to_parent.position_.x_ = io_skeleton->bone_length;
    io_skeleton->joints[LOW_LEFT_ARM].parent_index = UP_LEFT_ARM;
    io_skeleton->joints[LOW_LEFT_ARM].child_index = LEFT_HAND;
    io_skeleton->joints[LOW_LEFT_ARM].local_to_parent.position_.x_ = io_skeleton->bone_length;
    io_skeleton->joints[LEFT_HAND].parent_index = LOW_LEFT_ARM;
    io_skeleton->joints[LEFT_HAND].local_to_parent.position_.x_ = io_skeleton->bone_length;

    // Right arm
    io_skeleton->joints[UP_RIGHT_ARM].parent_index = TORSO;
    io_skeleton->joints[UP_RIGHT_ARM].child_index = LOW_RIGHT_ARM;
    io_skeleton->joints[UP_RIGHT_ARM].local_to_parent.position_.x_ = -io_skeleton->bone_length;
    io_skeleton->joints[LOW_RIGHT_ARM].parent_index = UP_RIGHT_ARM;
    io_skeleton->joints[LOW_RIGHT_ARM].child_index = RIGHT_HAND;
    io_skeleton->joints[LOW_RIGHT_ARM].local_to_parent.position_.x_ = -io_skeleton->bone_length;
    io_skeleton->joints[RIGHT_HAND].parent_index = LOW_RIGHT_ARM;
    io_skeleton->joints[RIGHT_HAND].local_to_parent.position_.x_ = -io_skeleton->bone_length;

    // Cache the pose
    for (uint8_t i = 0; i < io_skeleton->num_joints; ++i)
    {
        io_skeleton->cached_pose[i] = io_skeleton->joints[i].local_to_parent;
    }
}

void Skeleton::InitPalm(Skeleton*& io_skeleton)
{

}

} // namespace animation
} // namespace engine

