// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

layout(location = 0) in vec3 i_position;

uniform mat4 g_transform_view;
uniform mat4 g_transform_projection;

// Output
//=======

layout(location = 0) out vec3 o_uv;

// Entry Point
//============

void main()
{
	gl_Position = g_transform_projection * g_transform_view * vec4(i_position, 1.0);
	o_uv = normalize(i_position);
}