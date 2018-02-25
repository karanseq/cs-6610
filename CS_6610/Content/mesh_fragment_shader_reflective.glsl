// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

// #define USE_BLINN

// Input
//======

// The interpolated vertex position in camera space
layout(location = 0) in vec3 i_vertex;
// The interpolated normal in camera space
layout(location = 1) in vec3 i_normal;

// Texture parameters
layout(binding = 0) uniform samplerCube g_textureSampler;

// The camera position in world space
uniform vec3 g_cameraPosition;
// The light position in world space
uniform vec3 g_lightPosition;

uniform mat4 g_transform_view;

// Lighting parameters
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
vec3 getDiffuse(in vec3 lightDirection, in vec3 normal);
vec3 getSpecular(in vec3 lightDirection, in vec3 normal);
vec3 getAmbient();

// Main
//=====

void main()
{
	vec3 incidentEye = -vec3(normalize(inverse(g_transform_view) * vec4(i_vertex, 1.0)));
	vec3 normal = vec3(normalize(inverse(g_transform_view) * vec4(i_normal, 0.0)));
	vec3 reflected = reflect(incidentEye, normal);
	o_color = texture(g_textureSampler, reflected) * evaluateLights();
}

vec4 evaluateLights()
{
	vec3 lightDirection = normalize(g_lightPosition - i_vertex);
	vec3 normal = normalize(i_normal);

	return vec4(
		getDiffuse(lightDirection, normal) + 
		getSpecular(lightDirection, normal) + 
		getAmbient()
		, 1.0);
}

vec3 getDiffuse(in vec3 lightDirection, in vec3 normal)
{
	vec3 diffuse = clamp(dot(lightDirection, normal), 0.0, 1.0) * g_diffuse;
	return diffuse;
}

vec3 getSpecular(in vec3 lightDirection, in vec3 normal)
{
	vec3 viewDirection = normalize(i_vertex - g_cameraPosition);

#if defined USE_BLINN

	vec3 halfAngle = normalize(lightDirection + viewDirection);
	vec3 specular = vec3(g_specular * pow(clamp(dot(halfAngle, normal), 0.0, 1.0), g_shininess));

#else

	vec3 reflection = reflect(-lightDirection, normal);
	vec3 specular = vec3(g_specular * pow(max(dot(viewDirection, reflection), 0.0), g_shininess));

#endif

	return specular;
}

vec3 getAmbient()
{
	return g_ambient;
}
