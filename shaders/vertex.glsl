#version 410

layout(location = 0) in vec4 aPos;

out vec4 color;

uniform mat4 model;
uniform mat4 projection;
uniform vec3 aColor;

void main() {
	gl_Position = projection * model * aPos;

	color = vec4(aColor, 1.0f);
}