// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

#define USE_BLINN

// Input
//======

// The interpolated vertex position in camera space
layout(location = 0) in vec3 i_vertex;
// The interpolated normal in camera space
layout(location = 1) in vec3 i_normal;

// Color
uniform vec3 g_color;

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Main
//=====

void main()
{
	o_color = vec4(g_color, 1.0);
}
