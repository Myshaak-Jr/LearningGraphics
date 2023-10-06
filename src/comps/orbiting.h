#pragma once

#include <glm/glm.hpp>


namespace comps {
	struct orbiting {
		glm::vec3 duration;
		glm::vec3 amplitude;
		glm::vec3 phase;
		glm::vec3 center;
	};
}