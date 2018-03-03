// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

// Output
//=======

layout(location = 0) out float o_fragDepth;

// Main
//=====

void main()
{
	o_fragDepth = gl_FragCoord.z;
}
