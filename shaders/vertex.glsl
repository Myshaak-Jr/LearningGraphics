#version 410


layout(location = 0) in vec4 aPos;
layout(location = 1) in vec3 aNormal;

out vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normal;
uniform vec3 aColor;

struct DirLight {
	vec3 dir;
	vec3 diffuse;
};

#define MAX_DIR_LIGHTS 10
uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform uint numDirLights;

vec3 calcDirLight(DirLight light, vec3 viewNormal);

void main() {
	gl_Position = projection * view * model * aPos;

	vec3 result = vec3(0.0);

	vec3 viewNormal = normalize(normal * aNormal);

	for (uint i = 0; i < numDirLights; i++) {
		result += calcDirLight(dirLights[i], viewNormal);
	}

	//color = vec4(result, 1.0f) + vec4(aColor * 0.2f, 1.0f);
	color = vec4(aColor, 1.0f);
}

vec3 calcDirLight(DirLight light, vec3 viewNormal) {
	vec3 lightDir = normalize(-(view * vec4(light.dir, 0.0)).xyz);

	float diff = max(dot(viewNormal, lightDir), 0.0);
	
	vec3 diffuse = light.diffuse * diff * aColor;

	return diffuse;
}