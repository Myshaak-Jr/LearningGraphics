#pragma once

#include <glm/glm.hpp>


namespace comps {
	struct dynamicallyScaled {
		glm::vec3 duration;
		glm::vec3 start;
		glm::vec3 end;
	};
}