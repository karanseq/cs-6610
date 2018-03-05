// GLSL shaders require the version to be #defined before anything else in the shader
#version 420

#define USE_BLINN

// Input
//======

// The interpolated vertex position in view space
layout(location = 0) in vec3 i_vertexViewSpace;
// The interpolated normal in view space
layout(location = 1) in vec3 i_normal;
// The vertex position in light-clip space
layout(location = 2) in vec4 i_vertexLightSpace;

// The camera position in world space
uniform vec3 g_cameraPosition;
// The light position in world space
uniform vec3 g_lightPosition;

// Lighting parameters
uniform vec3 g_ambientLightIntensity;
uniform vec3 g_ambient;
uniform vec3 g_diffuse;
uniform vec3 g_specular;
uniform float g_shininess;

// Texture parameters
layout(binding = 0) uniform sampler2DShadow g_textureSampler;

// Color
uniform vec3 g_color;

// Output
//=======

// Whatever color value is output from the fragment shader
// will determine the color of the corresponding pixel on the screen
out vec4 o_color;

// Function Declarations
//======================

vec4 evaluateLights(float i_evaluatedShadow);
vec3 getDiffuse(in vec3 lightDirection, in vec3 normal);
vec3 getSpecular(in vec3 lightDirection, in vec3 normal);
vec3 getAmbient();
float evaluateShadows();

// Main
//=====

void main()
{
	float evaluatedShadow = evaluateShadows();
	o_color = vec4(g_color, 1.0) * evaluateLights(evaluatedShadow);
}

vec4 evaluateLights(float i_evaluatedShadow)
{
	vec3 lightDirection = normalize(g_lightPosition - i_vertexViewSpace);
	vec3 normal = normalize(i_normal);

	return vec4(
		getDiffuse(lightDirection, normal) * (1.0 - i_evaluatedShadow) + 
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
	vec3 viewDirection = normalize(i_vertexViewSpace - g_cameraPosition);

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

float evaluateShadows()
{
	vec3 projectedCoords = i_vertexLightSpace.xyz / i_vertexLightSpace.w;
	projectedCoords = projectedCoords * 0.5 + 0.5;

	float closestDepth = texture(g_textureSampler, projectedCoords.xyz);
	float currentDepth = projectedCoords.z - 0.05;
	float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	return shadow;
}