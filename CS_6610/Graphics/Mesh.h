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

// Util includes
#include "Utils/cyMatrix.h"

// Forward declarations

namespace cy {
    class GLSLProgram;
}

namespace engine {
namespace graphics {

enum class EMeshSelectionType : uint8_t
{
    None,
    Editable,
    NonEditable,
};

class MeshHelpers;

class Mesh
{
public:
    Mesh() = default;
    virtual ~Mesh();

    // Rendering
    //==========
public:
    void Render(cy::GLSLProgram& io_program);
    void Render(cy::GLSLProgram& io_program, const cy::Matrix4f& i_model);

private:
    uint16_t num_indices_ = 0;
    GLuint vertex_buffer_id_ = 0;
    GLuint index_buffer_id_ = 0;
    GLuint vertex_array_id_ = 0;

    // Selection
    //==========
public:
    void InitSelection(EMeshSelectionType i_selection_type);

    FORCEINLINE EMeshSelectionType GetSelectionType() const { return selection_type_; }

    FORCEINLINE bool GetIsSelected() const { return is_selected_; }
    FORCEINLINE void SetIsSelected(bool i_selected) { is_selected_ = i_selected; }

private:
    void CreateSelectionGizmo();

private:
    EMeshSelectionType selection_type_ = EMeshSelectionType::None;
    Mesh* selection_meshes_ = nullptr;
    bool is_selected_ = false;
    static constexpr uint8_t NUM_SELECTION_MESHES = 3;

    // Data
    //=====
public:
    FORCEINLINE const engine::math::Transform& GetTransform() const { return transform_; }
    FORCEINLINE engine::math::Transform& GetTransform() { return transform_; }
    FORCEINLINE void SetTransform(const engine::math::Transform& i_transform) { transform_ = i_transform; }

    FORCEINLINE const Color GetColor() const { return color_; }
    FORCEINLINE void SetColor(const Color& i_color) { color_ = i_color; }

    FORCEINLINE const Color GetSelectedColor() const { return selected_color_; }
    FORCEINLINE void SetSelectedColor(const Color& i_color) { selected_color_ = i_color; }

private:
    engine::math::Transform transform_;
    engine::graphics::Color color_;
    engine::graphics::Color selected_color_;

    friend class MeshHelpers;

}; // class Mesh

} // namesapce graphics
} // namespace engine

#endif // ENGINE_MESH_H_
