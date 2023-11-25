#version 410 core


layout(location = 0) in vec4 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Normal;
//out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normal;


void main() {
	vec4 tempInViewSpace = view * model * aPos;

	//FragPos = tempInViewSpace.xyz;
	gl_Position = proj * tempInViewSpace;
	Normal = normal * aNormal;
}

