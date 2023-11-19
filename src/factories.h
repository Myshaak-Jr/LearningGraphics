#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "modelManager/modelManager.h"
#include "color.h"

namespace factories {
	entt::entity createTree(
		const std::shared_ptr<entt::registry>& registry,
		const std::shared_ptr<ModelManager>& modelMngr,
		glm::vec3 pos, glm::vec3 scale
	);

	entt::entity createTemple(
		const std::shared_ptr<entt::registry>& registry,
		const std::shared_ptr<ModelManager>& modelMngr,
		glm::vec3 pos, glm::vec3 rot, glm::vec3 scale
	);

	entt::entity createSphere(
		const std::shared_ptr<entt::registry>& registry,
		const std::shared_ptr<ModelManager>& modelMngr,
		glm::vec3 pos, glm::vec3 rot, glm::vec3 scale,
		Color::RGB color
	);

	// applies first pitch than yaw than roll
	entt::entity createDirLight(
		const std::shared_ptr<entt::registry>& registry,
		const Color::RGB& color,
		float ambient, float diffuse, float specular,
		float yaw, float pitch, float roll
	);
}