#version 410 core

in vec2 TexCoords;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform vec2 resolution;
uniform vec3 bgColor;

const float depthThreshold = 0.02;
const float normalThreshold = 0.12;

out vec4 FragColor;


float checkDepthDifference(vec2 coord, vec2 texelSize)
{
	float currentDepth = texture(depthTexture, coord).r;

	float leftDepth = texture(depthTexture, coord + vec2(-texelSize.x, 0.0)).r;
	float rightDepth = texture(depthTexture, coord + vec2(texelSize.x, 0.0)).r;
	float bottomDepth = texture(depthTexture, coord + vec2(0.0, -texelSize.y)).r;
	float topDepth = texture(depthTexture, coord + vec2(0.0, texelSize.y)).r;
	
	float leftDiff = abs(currentDepth - leftDepth);
	float rightDiff = abs(currentDepth - rightDepth);
	float bottomDiff = abs(currentDepth - bottomDepth);
	float topDiff = abs(currentDepth - topDepth);

	return max(max(leftDiff, rightDiff), max(bottomDiff, topDiff));
}

float checkNormalDifference(vec2 coord, vec2 texelSize)
{
	vec3 currentNormal = texture(normalTexture, coord).rgb;

	vec3 leftNormal = texture(normalTexture, coord + vec2(-texelSize.x, 0.0)).rgb;
	vec3 rightNormal = texture(normalTexture, coord + vec2(texelSize.x, 0.0)).rgb;
	vec3 bottomNormal = texture(normalTexture, coord + vec2(0.0, -texelSize.y)).rgb;
	vec3 topNormal = texture(normalTexture, coord + vec2(0.0, texelSize.y)).rgb;
	
	float leftDiff = length(currentNormal - leftNormal);
	float rightDiff = length(currentNormal - rightNormal);
	float bottomDiff = length(currentNormal - bottomNormal);
	float topDiff = length(currentNormal - topNormal);

	return max(max(leftDiff, rightDiff), max(bottomDiff, topDiff));
}

bool isEdge(vec2 coord, vec2 texelSize) {
	float depthEdge = checkDepthDifference(coord, texelSize);
	float normalEdge = checkNormalDifference(coord, texelSize);

	return (depthEdge > depthThreshold) || (normalEdge > normalThreshold);
}

float luminance(vec3 color) {
	return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
	vec2 texelSize = 1.0 / resolution;

	vec3 phongColor = texture(colorTexture, TexCoords).rgb;
	float lum = luminance(phongColor);
	bool isSpecular = lum > 0.6;


	if (isEdge(TexCoords, texelSize) || isSpecular) {
		FragColor = vec4(phongColor, 1.0);
	}
	else {
		FragColor = vec4(bgColor, 1.0);
	}
}