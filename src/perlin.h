#pragma once

#include <vector>
#include <random>
#include <glm/glm.hpp>

namespace myMath {
	class Perlin {
	private:
		uint32_t seed;

		float calcAngle(glm::ivec2 pos) const;
		glm::vec2 calcGradientVec(glm::ivec2 pos) const;

	public:
		Perlin(uint32_t seed);
		~Perlin() = default;

		/* returns noise value in [0, 1] */
		float get(glm::vec2 position) const;
	};
}