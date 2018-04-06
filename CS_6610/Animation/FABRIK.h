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
    uint8_t                             iterations = 25;
    float                               tolerance = 0.01f;

}; // struct FABRIKParams

void FABRIK(const FABRIKParams& i_params);

void SolveForward(const FABRIKParams& i_params);
void SolveBackward(const FABRIKParams& i_params);
void UpdateRotations(const FABRIKParams& i_params);

float CalculateChainLength(const Skeleton* i_skeleton, uint16_t i_rootJointIndex, uint16_t i_endJointIndex);
void PrintSolvedJoints(const FABRIKParams& i_params);

} // namespace animation
} // namespace engine

#endif // ENGINE_FABRIK_H_
