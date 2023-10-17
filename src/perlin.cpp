#include "perlin.h"
#include "myMath.h"


myMath::Perlin::Perlin(uint32_t seed) : seed(seed) {}

float myMath::Perlin::calcAngle(glm::ivec2 pos) const {
	return (myMath::hashPos(pos, seed) / static_cast<float>(UINT32_MAX)) * 360.0f;
}

glm::vec2 myMath::Perlin::calcGradientVec(glm::ivec2 pos) const {
	float angle = calcAngle(pos);

	return glm::vec2(cos(angle), sin(angle));
}


float myMath::Perlin::get(glm::vec2 position) const {
	const glm::ivec2 top_left = glm::floor(position);

	float dotProducts[4] = {};
	const glm::ivec2 relCornerPositions[4] = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };

	for (int i = 0; i < 4; i++) {
		const glm::ivec2 cornerPos = top_left + relCornerPositions[i];
		const glm::vec2 gradient = calcGradientVec(cornerPos);

		dotProducts[i] = glm::dot(gradient, position - glm::vec2(cornerPos));
	}

	const glm::vec2 rel = position - glm::vec2(top_left);

	const float i1 = glm::mix(dotProducts[0], dotProducts[1], glm::smoothstep(0.0f, 1.0f, rel.x));
	const float i2 = glm::mix(dotProducts[2], dotProducts[3], glm::smoothstep(0.0f, 1.0f, rel.x));
	const float value = glm::mix(i1, i2, glm::smoothstep(0.0f, 1.0f, rel.y));
	return (value + 1.0f) * 0.5f;
}
