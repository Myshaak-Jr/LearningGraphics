#version 410 core

in vec2 TexCoords;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthTexture;
uniform vec2 resolution;

out vec4 FragColor;

void main() {
    vec3 col = texture(colorTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
}