#version 410 core

in vec2 TexCoord;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform vec2 resolution;
uniform vec3 bgColor;

uniform float zNear, zFar;

const float depthThreshold = 0.2;
const float normalThreshold = 0.2;

out vec4 FragColor;


float linearDepth(float z) {
    float z_n = 2.0 * z - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
    return z_e;
}

float depthSobel(vec2 coord, vec2 texelSize) {
	float top_left = linearDepth(texture(depthTexture, coord + vec2(-texelSize.x, texelSize.y)).r);
	float top_right = linearDepth(texture(depthTexture, coord + texelSize).r);
	float bottom_left = linearDepth(texture(depthTexture, coord + -texelSize).r);
	float bottom_right = linearDepth(texture(depthTexture, coord + vec2(texelSize.x, -texelSize.y)).r);
	
	float left = linearDepth(texture(depthTexture, coord + vec2(-texelSize.x, 0.0)).r);
	float right = linearDepth(texture(depthTexture, coord + vec2(texelSize.x, 0.0)).r);
	float bottom = linearDepth(texture(depthTexture, coord + vec2(0.0, -texelSize.y)).r);
	float top = linearDepth(texture(depthTexture, coord + vec2(0.0, texelSize.y)).r);
	
	// horizontal
	float gx = top_left + 2 * left + bottom_left - (top_right + 2 * right + bottom_right);
	gx /= 8;

	// vertical
	float gy = top_left + 2 * top + top_right - (bottom_left + 2 * bottom + bottom_right);
	gy /= 8;
	
	// result
	return length(vec2(gx, gy));
}

float normalSobel(vec2 coord, vec2 texelSize) {
	vec3 current = texture(normalTexture, coord).rgb * 2.0 - 1.0;

	float top_left = dot(current, texture(normalTexture, coord + vec2(-texelSize.x, texelSize.y)).rgb  * 2.0 - 1.0);
	float top_right = dot(current, texture(normalTexture, coord + texelSize).rgb  * 2.0 - 1.0);
	float bottom_left = dot(current, texture(normalTexture, coord + -texelSize).rgb  * 2.0 - 1.0);
	float bottom_right = dot(current, texture(normalTexture, coord + vec2(texelSize.x, -texelSize.y)).rgb  * 2.0 - 1.0);
	
	float left = dot(current, texture(normalTexture, coord + vec2(-texelSize.x, 0.0)).rgb  * 2.0 - 1.0);
	float right = dot(current, texture(normalTexture, coord + vec2(texelSize.x, 0.0)).rgb  * 2.0 - 1.0);
	float bottom = dot(current, texture(normalTexture, coord + vec2(0.0, -texelSize.y)).rgb  * 2.0 - 1.0);
	float top = dot(current, texture(normalTexture, coord + vec2(0.0, texelSize.y)).rgb  * 2.0 - 1.0);
	
	// horizontal
	float gx = top_left + 2 * left + bottom_left - (top_right + 2 * right + bottom_right);

	// vertical
	float gy = top_left + 2 * top + top_right - (bottom_left + 2 * bottom + bottom_right);
	
	// result
	return length(vec2(gx, gy));
}

bool isEdge(vec2 coord, vec2 texelSize) {
	float depthEdge = depthSobel(TexCoord, texelSize);
	float normalEdge = normalSobel(TexCoord, texelSize);

	return (depthEdge >= depthThreshold) || (normalEdge >= normalThreshold);
}

void main() {
	vec2 texelSize = 1.0 / resolution;

	if (isEdge(TexCoord, texelSize)) {
		FragColor = vec4(0.0);
	}
	else {
		FragColor = vec4(texture(colorTexture, TexCoord).rgb, 1.0);
	}
}