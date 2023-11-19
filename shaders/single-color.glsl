#version 410

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

out vec4 FragColor;
//layout(location = 0) out vec4 FragColor;
//layout(location = 1) out vec4 NormalColor;

in vec3 Normal;
in vec3 FragPos;

uniform Material material;

void main() {
	FragColor = vec4(material.diffuse, 1.0);
	//NormalColor = vec4(norm * 0.5 + 0.5, 1.0);
}