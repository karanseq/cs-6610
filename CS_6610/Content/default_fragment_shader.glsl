// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

void main()
{
	o_color = vec4(0.8, 0.2, 0.5, 1.0);
}