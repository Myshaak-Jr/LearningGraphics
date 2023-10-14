#pragma once

#include <vector>
#include <random>
#include <glm/glm.hpp>

class Perlin {
private:
	uint32_t seed;

	uint32_t hashPos(glm::ivec2 pos);
	float calcAngle(glm::ivec2 pos);
	glm::vec2 calcGradientVec(glm::ivec2 pos);

public:
	Perlin(uint32_t seed);
	~Perlin() = default;

	float get(glm::vec2 position);
	float get(float x, float y);
	glm::vec3 getNormal(glm::vec2 position);
	glm::vec3 getNormal(float x, float y);
};