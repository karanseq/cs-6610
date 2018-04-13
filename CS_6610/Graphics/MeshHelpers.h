#ifndef MESH_HELPERS_H_
#define MESH_HELPERS_H_

// Library includes
#include <stdint.h>

// GL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Forward declarations
namespace engine {
namespace math {
    class Vec3D;
}
namespace graphics {
    class Mesh;
}
}

namespace engine {
namespace graphics {

class MeshHelpers
{
public:
    static void CreatePlaneMesh(Mesh& o_mesh, float i_half_width);

    static void CreateBoxMesh(Mesh& o_mesh, float i_half_width);

    static void CreateArrowMesh(Mesh& o_mesh, float i_height, float i_width);

    static constexpr uint8_t NUM_INDICES_IN_PLANE = 6;
    static constexpr uint8_t NUM_INDICES_IN_BOX = 36;
    static constexpr uint8_t NUM_INDICES_IN_ARROW = 9;

private:
    MeshHelpers() = delete;
    ~MeshHelpers() = delete;

    MeshHelpers(const MeshHelpers&) = delete;
    MeshHelpers& operator=(const MeshHelpers&) = delete;

}; // class MeshHelpers

} // namespace graphics
} // namespace engine

#endif // MESH_HELPERS_H_
