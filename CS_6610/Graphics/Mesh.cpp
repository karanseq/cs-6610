#include "Mesh.h"

// Util includes
#include "Utils/cyGL.h"
#include "Utils/cyMatrix.h"

namespace engine {
namespace graphics {

void Mesh::Render(cy::GLSLProgram& io_program)
{
    // Set the model transformation
    cy::Matrix4f model;
    cy::Matrix4f::GetMatrixFromTransform(model, transform_);
    io_program.SetUniformMatrix4("g_transform_model", model.data);

    // Set the color
    io_program.SetUniform("g_color", color_.r, color_.g, color_.b);

    // Draw
    glBindVertexArray(vertex_array_id_);
    glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_BYTE, 0);
}

void Mesh::Render(cy::GLSLProgram& io_program, const float* i_model)
{
    // Set the model transformation
    io_program.SetUniformMatrix4("g_transform_model", i_model);

    // Set the color
    io_program.SetUniform("g_color", color_.r, color_.g, color_.b);

    // Draw
    glBindVertexArray(vertex_array_id_);
    glDrawElements(GL_TRIANGLES, num_indices_, GL_UNSIGNED_BYTE, 0);
}

} // namespace graphics
} // namespace engine
