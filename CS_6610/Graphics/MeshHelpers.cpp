#include "MeshHelpers.h"

// Library includes
#include <stdint.h>

// Engine includes
#include "Graphics/Mesh.h"
#include "Math/Vec3D.h"

namespace engine {
namespace graphics {

void MeshHelpers::CreatePlaneMesh(Mesh& o_mesh, float i_half_width)
{
    // Create a vertex array object and make it active
    {
        constexpr GLsizei array_count = 1;
        glGenVertexArrays(array_count, &o_mesh.vertex_array_id_);
        glBindVertexArray(o_mesh.vertex_array_id_);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei buffer_count = 1;
        glGenBuffers(buffer_count, &o_mesh.vertex_buffer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, o_mesh.vertex_buffer_id_);
    }

    // Assign data to the vertex buffer
    {
        constexpr uint8_t num_vertices = 4;

        const engine::math::Vec3D vertices[num_vertices] = {
            engine::math::Vec3D(-i_half_width, 0.0f, i_half_width),       // bottom-left
            engine::math::Vec3D(i_half_width, 0.0f, i_half_width),        // bottom-right
            engine::math::Vec3D(i_half_width, 0.0f, -i_half_width),       // top-right
            engine::math::Vec3D(-i_half_width, 0.0f, -i_half_width)       // top-left
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertex_element_location = 0;
        constexpr GLuint element_count = 3;
        glVertexAttribPointer(vertex_element_location, element_count, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertex_element_location);
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei buffer_count = 1;
        glGenBuffers(buffer_count, &o_mesh.index_buffer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_mesh.index_buffer_id_);
    }

    // Assign data to the index buffer
    {
        o_mesh.num_indices_ = NUM_INDICES_IN_PLANE;
        constexpr uint8_t indices[NUM_INDICES_IN_PLANE] = { 0, 1, 2, 0, 2, 3 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
}

void MeshHelpers::CreateBoxMesh(Mesh& o_mesh, float i_half_width)
{
    // Create a vertex array object and make it active
    {
        constexpr GLsizei array_count = 1;
        glGenVertexArrays(array_count, &o_mesh.vertex_array_id_);
        glBindVertexArray(o_mesh.vertex_array_id_);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei buffer_count = 1;
        glGenBuffers(buffer_count, &o_mesh.vertex_buffer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, o_mesh.vertex_buffer_id_);
    }

    // Assign data to the vertex buffer
    {
        constexpr uint8_t num_vertices = 8;

        const engine::math::Vec3D vertices[num_vertices] = {
            engine::math::Vec3D(-i_half_width, -i_half_width, -i_half_width),
            engine::math::Vec3D( i_half_width, -i_half_width, -i_half_width),
            engine::math::Vec3D( i_half_width,  i_half_width, -i_half_width),
            engine::math::Vec3D(-i_half_width,  i_half_width, -i_half_width),
            engine::math::Vec3D(-i_half_width, -i_half_width,  i_half_width),
            engine::math::Vec3D( i_half_width, -i_half_width,  i_half_width),
            engine::math::Vec3D( i_half_width,  i_half_width,  i_half_width),
            engine::math::Vec3D(-i_half_width,  i_half_width,  i_half_width)
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertex_element_location = 0;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertex_element_location, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertex_element_location);
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei buffer_count = 1;
        glGenBuffers(buffer_count, &o_mesh.index_buffer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_mesh.index_buffer_id_);
    }

    // Assign data to the index buffer
    {
        o_mesh.num_indices_ = NUM_INDICES_IN_BOX;
        constexpr uint8_t indices[NUM_INDICES_IN_BOX] = {
            // front
            0, 1, 2,
            0, 2, 3,
            // back
            5, 4, 7,
            5, 7, 6,
            // left
            4, 0, 3,
            4, 3, 7,
            // right
            1, 5, 6,
            1, 6, 2,
            // top
            3, 2, 6,
            3, 6, 7,
            // bottom
            2, 6, 1,
            2, 1, 0
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
}

void MeshHelpers::CreateArrowMesh(Mesh& o_mesh, float i_height, float i_width)
{
    // Create a vertex array object and make it active
    {
        constexpr GLsizei array_count = 1;
        glGenVertexArrays(array_count, &o_mesh.vertex_array_id_);
        glBindVertexArray(o_mesh.vertex_array_id_);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei buffer_count = 1;
        glGenBuffers(buffer_count, &o_mesh.vertex_buffer_id_);
        glBindBuffer(GL_ARRAY_BUFFER, o_mesh.vertex_buffer_id_);
    }

    // Assign data to the vertex buffer
    {
        constexpr uint8_t num_vertices = 7;

        const float quarter_width = i_width * 0.25f;

        const engine::math::Vec3D vertices[num_vertices] = {
            engine::math::Vec3D(-quarter_width, 0.0f, 0.0f),
            engine::math::Vec3D(quarter_width, 0.0f, 0.0f),
            engine::math::Vec3D(quarter_width, i_height * 0.8f, 0.0f),
            engine::math::Vec3D(-quarter_width, i_height * 0.8f, 0.0f),
            engine::math::Vec3D(-quarter_width * 2.0f, i_height * 0.8f, 0.0f),
            engine::math::Vec3D(quarter_width * 2.0f, i_height * 0.8f, 0.0f),
            engine::math::Vec3D(0.0f, i_height, 0.0f)
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertex_element_location = 0;
        constexpr GLuint element_count = 3;
        glVertexAttribPointer(vertex_element_location, element_count, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertex_element_location);
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei buffer_count = 1;
        glGenBuffers(buffer_count, &o_mesh.index_buffer_id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_mesh.index_buffer_id_);
    }

    // Assign data to the index buffer
    {
        o_mesh.num_indices_ = NUM_INDICES_IN_ARROW;
        constexpr uint8_t indices[NUM_INDICES_IN_ARROW] = { 0, 1, 2, 0, 2, 3, 4, 5, 6 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
}

} // namespace graphics
} // namespace engine

