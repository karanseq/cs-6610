// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec2 i_uv;

uniform mat4 g_transform_model;
uniform mat4 g_transform_view;
uniform mat4 g_transform_projection;

// Output
//=======

layout(location = 0) out vec2 o_uv;

// Entry Point
//============

void main()
{
	gl_Position = g_transform_projection * g_transform_model * vec4(i_position, 1.0);
	o_uv = i_uv;
}