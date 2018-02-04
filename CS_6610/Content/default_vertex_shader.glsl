// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;

uniform mat4 g_transform_modelViewProjection;
uniform mat3 g_transform_modelView;

// Output
//=======

layout(location = 0) out vec3 o_vertex;
layout(location = 1) out vec3 o_normal;

// Entry Point
//============

void main()
{
	gl_Position = g_transform_modelViewProjection * vec4(i_position, 1.0);
	o_vertex = g_transform_modelView * i_position;
	o_normal = transpose(inverse(g_transform_modelView)) * i_normal;
}