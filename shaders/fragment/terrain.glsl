#version 410

in float height;

out vec4 fragColor;

void main() {
	fragColor = vec4(0.0f, mix(0.3f, 1.0f, height), 0.0f, 1.0f);
}