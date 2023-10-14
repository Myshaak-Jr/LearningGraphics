#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../constants.h"

namespace comps {
	struct orientation {		
		glm::quat orient;

		glm::vec3 center;

		inline orientation(float angle, glm::vec3 axis, glm::vec3 center = glm::vec3(0.0f)) : center(center) {
			orient = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
		}

		inline orientation(float yaw, float pitch, float roll, glm::vec3 center = glm::vec3(0.0f)) : center(center) {
			glm::quat y = glm::angleAxis(glm::radians(yaw), VEC_UP);
			glm::quat x = glm::angleAxis(glm::radians(pitch), VEC_RIGHT);
			glm::quat z = glm::angleAxis(glm::radians(roll), VEC_FORWARD);

			orient = z * x * y;
		}

		inline orientation() : orientation(0.0f, VEC_UP, glm::vec3(0.0f)) {}
	};
}