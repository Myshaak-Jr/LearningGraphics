#pragma once

#include <glm/glm.hpp>


namespace comps {
	struct rotation {
		/* TODO: change to quaternions to avoid that bitch known as gimbal lock (brumbálův lok) */
		
		float yaw;
		float pitch;
		float roll;

		glm::vec3 center;
	};
}