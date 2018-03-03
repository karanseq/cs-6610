// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

// The interpolated uvs
layout(location = 0) in vec3 i_position;

// Texture parameters
layout(binding = 0) uniform sampler2D g_textureSampler;

uniform mat4 g_transform_model;
uniform mat4 g_transform_viewReflected;
uniform mat4 g_transform_projection;

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
	vec4 positionClipSpace = g_transform_projection * g_transform_viewReflected * g_transform_model * vec4(i_position, 1.0);
	vec2 uv = vec2(positionClipSpace.x / positionClipSpace.w, positionClipSpace.y / positionClipSpace.w) * 0.5 + 0.5;
	o_color = texture(g_textureSampler, uv);
}
