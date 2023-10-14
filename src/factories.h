#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "modelManager.h"
#include "programManager.h"
#include "color.h"

namespace factories {
	entt::entity createTree(
		const std::unique_ptr<entt::registry>& registry,
		const std::unique_ptr<ModelManager>& modelMngr,
		const std::unique_ptr<ProgramManager>& prgMngr,
		glm::vec3 pos, glm::vec3 scale
	);

	entt::entity createTemple(
		const std::unique_ptr<entt::registry>& registry,
		const std::unique_ptr<ModelManager>& modelMngr,
		const std::unique_ptr<ProgramManager>& prgMngr,
		glm::vec3 pos, glm::vec3 rot, glm::vec3 scale
	);

	entt::entity createDirLight(
		const std::unique_ptr<entt::registry>& registry,
		const color::RGB& color,
		const glm::vec3& rot
	);
}