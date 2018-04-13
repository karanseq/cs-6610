#include "Mesh.h"

// Engine includes
#include "Graphics/MeshHelpers.h"

// Util includes
#include "Utils/cyGL.h"

namespace engine {
namespace graphics {

Mesh::~Mesh()
{
    if (selection_meshes_ != nullptr)
    {
        delete[] selection_meshes_;
        selection_meshes_ = nullptr;
    }
}

// Rendering
//==========

void Mesh::Render(cy::GLSLProgram& io_program)
{
    // Set the model transformation
    cy::Matrix4f model;
    cy::Matrix4f::GetMatrixFromTransform(model, transform_);
    io_program.SetUniformMatrix4("g_transform_model", model.data);

    // Set the color
    if (is_selected_ && selection_type_ != EMeshSelectionType::None)
    {
        io_program.SetUniform("g_color", selected_color_.r, selected_color_.g, selected_color_.b);
    }
    else
    {
        io_program.SetUniform("g_color", color_.r, color_.g, color_.b);
    }

    // Draw
    glBindVertexArray(vertex_array_id_);
    glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_BYTE, 0);

    // Draw selection
    if (is_selected_ && selection_type_ == EMeshSelectionType::Editable)
    {
        for (uint8_t i = 0; i < NUM_SELECTION_MESHES; ++i)
        {
            cy::Matrix4f arrow;
            cy::Matrix4f::GetMatrixFromTransform(arrow, selection_meshes_[i].GetTransform());
            arrow = model * arrow;
            selection_meshes_[i].Render(io_program, arrow);
        }
    }
}

void Mesh::Render(cy::GLSLProgram& io_program, const cy::Matrix4f& i_model)
{
    // Set the model transformation
    io_program.SetUniformMatrix4("g_transform_model", i_model.data);

    // Set the color
    if (is_selected_ && selection_type_ != EMeshSelectionType::None)
    {
        io_program.SetUniform("g_color", selected_color_.r, selected_color_.g, selected_color_.b);
    }
    else
    {
        io_program.SetUniform("g_color", color_.r, color_.g, color_.b);
    }

    // Draw
    glBindVertexArray(vertex_array_id_);
    glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_BYTE, 0);

    // Draw selection
    if (is_selected_ && selection_type_ == EMeshSelectionType::Editable)
    {
        for (uint8_t i = 0; i < NUM_SELECTION_MESHES; ++i)
        {
            cy::Matrix4f arrow;
            cy::Matrix4f::GetMatrixFromTransform(arrow, selection_meshes_[i].GetTransform());
            arrow = i_model * arrow;
            selection_meshes_[i].Render(io_program);
        }
    }
}

// Selection
//==========

void Mesh::InitSelection(EMeshSelectionType i_selection_type)
{
    if (selection_type_ == i_selection_type)
    {
        return;
    }

    selection_type_ = i_selection_type;

    if (selection_type_ == EMeshSelectionType::Editable)
    {
        CreateSelectionGizmo();
    }
}

void Mesh::CreateSelectionGizmo()
{
    if (selection_meshes_ != nullptr)
    {
        return;
    }

    selection_meshes_ = new Mesh[NUM_SELECTION_MESHES];
    constexpr float arrow_height = 2.5f;
    constexpr float arrow_width = 0.5f;

    // X-axis
    {
        MeshHelpers::CreateArrowMesh(selection_meshes_[0], arrow_height, arrow_width);

        engine::math::Transform transform;
        transform.rotation_ = engine::math::Quaternion(M_PI * -0.5f, engine::math::Vec3D::UNIT_Z);
        selection_meshes_[0].SetTransform(transform);

        selection_meshes_[0].SetColor(Color::RED);
        selection_meshes_[0].SetColor(Color::RED);
    }

    // Y-axis
    {
        MeshHelpers::CreateArrowMesh(selection_meshes_[1], arrow_height, arrow_width);

        engine::math::Transform transform;
        transform.rotation_ = engine::math::Quaternion(M_PI, engine::math::Vec3D::UNIT_Z);
        selection_meshes_[1].SetTransform(transform);

        selection_meshes_[1].SetColor(Color::GREEN);
        selection_meshes_[1].SetColor(Color::GREEN);
    }

    // Z-axis
    {
        MeshHelpers::CreateArrowMesh(selection_meshes_[2], arrow_height, arrow_width);

        engine::math::Transform transform;
        transform.rotation_ = engine::math::Quaternion(M_PI * -0.5f, engine::math::Vec3D::UNIT_X);
        selection_meshes_[2].SetTransform(transform);

        selection_meshes_[2].SetColor(Color::BLUE);
        selection_meshes_[2].SetColor(Color::BLUE);
    }
}

} // namespace graphics
} // namespace engine
