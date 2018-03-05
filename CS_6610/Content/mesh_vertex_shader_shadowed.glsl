// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

// The vertex position in model space
layout(location = 0) in vec3 i_position;
// The vertex normal in model space
layout(location = 1) in vec3 i_normal;

uniform mat4 g_transform_model;
uniform mat4 g_transform_view;
uniform mat4 g_transform_projection;
uniform mat4 g_transform_light;
uniform mat4 g_transform_lightProjection;

// Output
//=======

// The vertex position in view space
layout(location = 0) out vec3 o_vertexViewSpace;
// The vertex normal in view space
layout(location = 1) out vec3 o_normal;
// The vertex position in light space
layout(location = 2) out vec4 o_vertexLightSpace;

// Entry Point
//============

void main()
{
	// Regular MVP transformation
	gl_Position = g_transform_projection * g_transform_view * g_transform_model * vec4(i_position, 1.0);

	// Transform vertex and vertex normal into view space
	mat3 transform_modelView = mat3(g_transform_view * g_transform_model);
	o_vertexViewSpace = transform_modelView * i_position;
	o_normal = transpose(inverse(mat3(transform_modelView))) * i_normal;

	// Transform vertex into light-clip space
	o_vertexLightSpace = g_transform_lightProjection * g_transform_light * g_transform_model * vec4(i_position, 1.0);
}