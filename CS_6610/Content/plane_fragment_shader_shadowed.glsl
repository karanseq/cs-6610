// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

// The interpolated uvs
layout(location = 0) in vec4 i_position;

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
	// vec3 projectedCoords = i_position.xyz / i_position.w;
	// // projectedCoords = projectedCoords * 0.5 + 0.5;

	// float closestDepth = texture(g_textureSampler, projectedCoords.xyz).r;
	// float currentDepth = i_position.z;
	// float shadow = currentDepth > closestDepth ? 1.0 : 0.25;

	// o_color = vec4(0.5, 0.5, 0.5, 1.0);// * (1.0 - shadow);

	vec3 projectedCoords = i_position.xyz / i_position.w;
	float closestDepth = texture(g_textureSampler, projectedCoords.xy).r * 0.5;
	o_color = vec4(closestDepth, 0, 0, 1.0);
}
