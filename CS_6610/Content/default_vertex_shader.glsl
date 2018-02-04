// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;

uniform mat4 g_transform_model;
uniform mat4 g_transform_view;
uniform mat4 g_transform_projection;

// Output
//=======

layout(location = 0) out vec3 o_vertex;
layout(location = 1) out vec3 o_normal;

// Entry Point
//============

void main()
{
	gl_Position = g_transform_projection * g_transform_view * g_transform_model * vec4(i_position, 1.0);
	mat3 transform_modelView = mat3(g_transform_view * g_transform_model);
	o_vertex = transform_modelView * i_position;
	o_normal = transpose(inverse(mat3(transform_modelView))) * i_normal;
}