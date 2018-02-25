// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// #define USE_BLINN

// Input
//======

// The interpolated uvs
layout(location = 0) in vec3 i_uv;

// Texture parameters
layout(binding = 0) uniform samplerCube g_textureSampler;

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Function Declarations
//======================

// Main
//=====

void main()
{
	o_color = texture(g_textureSampler, i_uv);
}
