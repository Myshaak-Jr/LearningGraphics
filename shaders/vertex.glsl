#version 410

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 aColor;

out vec4 color;

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;


void main() {
	vec4 cameraPos = modelToCameraMatrix * aPos;
	gl_Position = cameraToClipMatrix * cameraPos;
	//gl_Position = cameraPos;
	color = aColor;
}