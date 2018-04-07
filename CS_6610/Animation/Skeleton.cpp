#include "Skeleton.h"

namespace engine {
namespace animation {

void Skeleton::CreateSkeleton(Skeleton*& io_skeleton, ESkeletonType i_type)
{
    if (io_skeleton == nullptr ||
        io_skeleton->num_joints == 0)
    {
        return;
    }

    // Allocate joints
    io_skeleton->joints = new engine::animation::Joint[io_skeleton->num_joints];
    io_skeleton->local_to_world_transforms = new cy::Matrix4f[io_skeleton->num_joints];
    io_skeleton->world_to_local_transforms = new cy::Matrix4f[io_skeleton->num_joints];
    io_skeleton->local_to_world_rotations = new engine::math::Quaternion[io_skeleton->num_joints];
    io_skeleton->world_to_local_rotations = new engine::math::Quaternion[io_skeleton->num_joints];
    io_skeleton->solved_joints = new engine::math::Vec3D[io_skeleton->num_joints];

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
    io_skeleton->joints[0].local_to_parent.rotation_ = engine::math::Quaternion::FORWARD * engine::math::Quaternion::FORWARD;
    io_skeleton->joints[0].local_to_parent.position_ = engine::math::Vec3D::ZERO;

    // Chain
    for (uint16_t i = 1; i < io_skeleton->num_joints; ++i)
    {
        io_skeleton->joints[i].local_to_parent.rotation_ = engine::math::Quaternion::FORWARD * engine::math::Quaternion::FORWARD;
        io_skeleton->joints[i].local_to_parent.position_.set(0.0f, io_skeleton->bone_length, 0.0f);
    }
}

void Skeleton::InitHumanoid(Skeleton*& io_skeleton)
{
    // Reset the transforms
    for (uint16_t i = 0; i < io_skeleton->num_joints; ++i)
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
}

void Skeleton::InitPalm(Skeleton*& io_skeleton)
{

}

} // namespace animation
} // namespace engine

