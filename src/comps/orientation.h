#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../constants.h"

namespace comps {
	struct orientation {		
		glm::quat orient;

		glm::vec3 center;

		inline orientation(glm::quat orient, glm::vec3 center = glm::vec3(0.0f)) : orient(orient), center(center) {}
		inline orientation() : orient(angleAxis(0.0f, VEC_UP)), center(glm::vec3(0.0f)) {}
	};
}