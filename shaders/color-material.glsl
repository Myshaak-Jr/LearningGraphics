#version 410

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

vec3 calcDirLight(DirLight light, vec3 viewDir, vec3 normal);

void main() {
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(-FragPos);

	vec3 result = vec3(0.0);

	for (uint i = 0; i < numDirLights; i++) {
		result += calcDirLight(dirLights[i], viewDir, norm);
	}

	FragColor = vec4(result, 1.0);
	NormalColor = vec4(norm * 0.5 + 0.5, 1.0);
}

vec3 calcDirLight(DirLight light, vec3 viewDir, vec3 normal) {
	// convert light direction to view space
	vec3 lightDir = normalize(-(view * vec4(light.dir, 0.0)).xyz);
	
	// calculate diffuse intensity
	float diff = dot(normal, lightDir) * 0.5 + 0.5;
	diff = pow(diff, 1.0 / 2.0);
	diff = (diff + 0.3) / (1.0 + 0.3);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	vec3 ambient = light.ambient * material.ambient;
	vec3 diffuse = light.diffuse * diff * material.diffuse;
	vec3 specular = light.specular * spec * material.specular;

	return ambient + diffuse + specular;
}