#ifndef ENGINE_FABRIK_H_
#define ENGINE_FABRIK_H_

// Library includes
#include <stdint.h>

// Engine includes
#include "Math/Vec3D.h"

namespace engine {
namespace animation {

struct Joint;
struct Skeleton;

struct FABRIKParams
{
    engine::math::Vec3D                 target = engine::math::Vec3D::ZERO;
    Skeleton*                           skeleton = nullptr;
    uint16_t                            root_joint_index = 0;
    uint16_t                            end_joint_index = 0;
    uint8_t                             i_iterations = 10;

}; // struct FABRIKParams

void FABRIK(const FABRIKParams& i_params);

void SolveForward(Skeleton* io_skeleton, const engine::math::Vec3D& i_target);
void SolveBackward(Skeleton* io_skeleton, const engine::math::Vec3D& i_target);

float CalculateChainLength(const Skeleton* i_skeleton, uint16_t i_rootJointIndex, uint16_t i_endJointIndex);

} // namespace animation
} // namespace engine

#endif // ENGINE_FABRIK_H_
