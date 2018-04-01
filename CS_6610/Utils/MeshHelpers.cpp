#include "MeshHelpers.h"

// Library includes
#include <stdint.h>

// Util includes
#include "Math/Vec3D.h"

// Static member initialization
const uint8_t MeshHelpers::NUM_INDICES_IN_PLANE = 6;
const uint8_t MeshHelpers::NUM_INDICES_IN_BOX = 36;

void MeshHelpers::CreatePlaneMesh(BufferIdGroup& o_bufferIds,
    float i_halfWidth)
{
    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &o_bufferIds.vertexArrayId);
        glBindVertexArray(o_bufferIds.vertexArrayId);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &o_bufferIds.vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, o_bufferIds.vertexBufferId);
    }

    // Assign data to the vertex buffer
    {
        constexpr uint8_t numVertices = 4;

        const engine::math::Vec3D vertices[numVertices] = {
            engine::math::Vec3D(-i_halfWidth, 0.0f, i_halfWidth),       // bottom-left
            engine::math::Vec3D(i_halfWidth, 0.0f, i_halfWidth),        // bottom-right
            engine::math::Vec3D(i_halfWidth, 0.0f, -i_halfWidth),       // top-right
            engine::math::Vec3D(-i_halfWidth, 0.0f, -i_halfWidth)       // top-left
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertexElementLocation = 0;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertexElementLocation);
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &o_bufferIds.indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_bufferIds.indexBufferId);
    }

    // Assign data to the index buffer
    {
        constexpr uint8_t indices[NUM_INDICES_IN_PLANE] = { 0, 1, 2, 0, 2, 3 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
}

void MeshHelpers::CreateBoxMesh(BufferIdGroup& o_bufferIds,
    float i_halfWidth)
{
    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &o_bufferIds.vertexArrayId);
        glBindVertexArray(o_bufferIds.vertexArrayId);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &o_bufferIds.vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, o_bufferIds.vertexBufferId);
    }

    // Assign data to the vertex buffer
    {
        constexpr uint8_t numVertices = 8;

        const engine::math::Vec3D vertices[numVertices] = {
            engine::math::Vec3D(-i_halfWidth, -i_halfWidth, -i_halfWidth),
            engine::math::Vec3D( i_halfWidth, -i_halfWidth, -i_halfWidth),
            engine::math::Vec3D( i_halfWidth,  i_halfWidth, -i_halfWidth),
            engine::math::Vec3D(-i_halfWidth,  i_halfWidth, -i_halfWidth),
            engine::math::Vec3D(-i_halfWidth, -i_halfWidth,  i_halfWidth),
            engine::math::Vec3D( i_halfWidth, -i_halfWidth,  i_halfWidth),
            engine::math::Vec3D( i_halfWidth,  i_halfWidth,  i_halfWidth),
            engine::math::Vec3D(-i_halfWidth,  i_halfWidth,  i_halfWidth)
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertexElementLocation = 0;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertexElementLocation);
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &o_bufferIds.indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_bufferIds.indexBufferId);
    }

    // Assign data to the index buffer
    {
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
