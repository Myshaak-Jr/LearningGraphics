#pragma once

#include <glm/glm.hpp>
#include "../color.h"

namespace comps {
	struct dirLight {
		color::RGB diffuse;
		glm::vec3 baseDir;

		inline dirLight(color::RGB diffuse, glm::vec3 baseDir = VEC_DOWN) : diffuse(diffuse), baseDir(baseDir) {}
	};
}