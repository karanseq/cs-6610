// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// Input
//======

layout(location = 0) in vec3 i_vertex;
layout(location = 1) in vec3 i_normal;

uniform vec3 g_cameraPosition;

uniform vec3 g_lightPosition;
uniform vec3 g_lightIntensity;
uniform vec3 g_ambientLightIntensity;

uniform vec3 g_ambient;
uniform vec3 g_diffuse;
uniform vec3 g_specular;
uniform float g_shininess;

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Function Declarations
//======================

vec4 evaluateLights();
vec4 getDiffuse();
vec4 getAmbientLight();

// Main
//=====

void main()
{
	vec4 color = vec4(1.0, 0.0f, 0.0, 1.0);	

	vec4 lights = evaluateLights();
	o_color = lights * color;

	//vec3 normalized = normalize(i_normal);
	//o_color = vec4(normalized, 1.0);
}

vec4 evaluateLights()
{
	vec3 lightDirection = normalize(g_lightPosition - i_vertex);
	vec3 normal = normalize(i_normal);

	float cosTheta = dot(lightDirection, normal);
	cosTheta = cosTheta < 0.0 ? 0.0 : cosTheta;

	return (getDiffuse() * cosTheta + getAmbientLight());
}

vec4 getDiffuse()
{
	vec4 diffuse = vec4(g_diffuse * g_lightIntensity, 1.0);
	return(diffuse);
}

vec4 getAmbientLight()
{
	vec4 ambient = vec4(g_ambient * g_ambientLightIntensity, 1.0);
	return(ambient);
}
