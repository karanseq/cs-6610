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
}

struct BufferIdGroup
{
    GLuint vertexArrayId = -1;
    GLuint vertexBufferId = -1;
    GLuint normalBufferId = -1;
    GLuint texCoordId = -1;
    GLuint indexBufferId = -1;
};

class MeshHelpers
{
public:
    static void CreatePlaneMesh(BufferIdGroup& o_bufferIds,
        float i_halfWidth);

    static void CreateBoxMesh(BufferIdGroup& o_bufferIds,
        float i_halfWidth);

    static const uint8_t NUM_INDICES_IN_PLANE;
    static const uint8_t NUM_INDICES_IN_BOX;

private:
    MeshHelpers() = delete;
    ~MeshHelpers() = delete;

    MeshHelpers(const MeshHelpers&) = delete;
    MeshHelpers& operator=(const MeshHelpers&) = delete;

}; // class MeshHelpers


#endif // MESH_HELPERS_H_
