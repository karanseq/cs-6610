// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

layout(location = 0) in vec3 i_position;

uniform mat4 g_transform_modelViewProjection;

// Output
//=======

// Entry Point
//============

void main()
{
	gl_Position = g_transform_modelViewProjection * vec4(i_position, 1.0);
}