#ifndef ENGINE_MESH_H_
#define ENGINE_MESH_H_

// Library includes
#include <stdint.h>

// GL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Engine includes
#include "Graphics/Color.h"
#include "Math/Transform.h"

// Forward declarations

namespace cy {
    class GLSLProgram;
}

namespace engine {
namespace graphics {

class MeshHelpers;

class Mesh
{
public:
    Mesh() = default;
    ~Mesh() = default;

    void Render(cy::GLSLProgram& io_program);
    void Render(cy::GLSLProgram& io_program, const float* i_model);

    FORCEINLINE const engine::math::Transform& GetTransform() const { return transform_; }
    FORCEINLINE engine::math::Transform& GetTransform() { return transform_; }
    FORCEINLINE void SetTransform(const engine::math::Transform& i_transform) { transform_ = i_transform; }

    FORCEINLINE const Color GetColor() const { return color_; }
    FORCEINLINE void SetColor(const Color& i_color) { color_ = i_color; }

private:
    engine::math::Transform transform_;
    engine::graphics::Color color_;

    uint16_t num_indices_ = 0;

    GLuint vertex_buffer_id_ = 0;
    GLuint index_buffer_id_ = 0;
    GLuint vertex_array_id_ = 0;

    friend class MeshHelpers;

}; // class Mesh

} // namesapce graphics
} // namespace engine

#endif // ENGINE_MESH_H_
