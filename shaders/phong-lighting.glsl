#version 410 core

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct DirLight {
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;

in vec3 Normal;
in vec3 FragPos;

uniform Material material;
uniform mat4 view;

#define MAX_DIR_LIGHTS 10
uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform uint numDirLights;

vec3 calcDirLightDiffuse(vec3 lightDiffuse, vec3 lightDir, vec3 normal);
vec3 calcDirLightSpecular(vec3 lightSpecular, vec3 lightDir, vec3 viewDir, vec3 normal);

float luminance(vec3 color) {
	return dot(color, vec3(0.299, 0.587, 0.114));
}

void main() {
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(-FragPos);

	float overallLightDiffuseLuminance = 0.0;
	//vec3 overallLightDiffuseColor = vec3(0.0);
	float overallLightSpecularLuminance = 0.0;
	//vec3 overallLightSpecularColor = vec3(0.0);
	vec3 overallLightAmbient = vec3(0.0);

	for (uint i = 0; i < numDirLights; i++) {
		vec3 lightDir = normalize(-(view * vec4(dirLights[i].dir, 0.0)).xyz);

		float diff = max(dot(norm, lightDir), 0.0);

		//overallLightDiffuseColor += dirLights[i].diffuse;
		overallLightDiffuseLuminance += luminance(dirLights[i].diffuse) * diff;

		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
		//overallLightSpecularColor += dirLights[i].specular;
		overallLightSpecularLuminance += luminance(dirLights[i].specular) * spec;

		overallLightAmbient += dirLights[i].ambient * material.ambient;
	}


	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

	const float numShades = 10.0;
	const float luminanceExponent = 0.7;
	
	//float intensity = floor(pow(overallLightDiffuseLuminance) * numShades) / numShades;

	//intensity = pow(intensity, 0.5);

	//vec3 result = mix(vec3(0.0), material.diffuse, intensity) + overallLightAmbient;

	FragColor = vec4(vec3(pow(overallLightDiffuseLuminance, luminanceExponent)), 1.0);
	NormalColor = vec4((1.0 + norm) * 0.5, 1.0);
}
