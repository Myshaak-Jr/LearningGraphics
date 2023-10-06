#pragma once

#include <glm/glm.hpp>


namespace comps {
	struct scale {
		glm::vec3 scl;

		inline scale(glm::vec3 scale = glm::vec3(1.0f)) : scl(scale) {}
		inline scale(float scale) : scl(scale) {}
	};
}