#include "factories.h"

#include <sstream>

#include "comps/position.h"
#include "comps/orientation.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/rotatedByKeyboard.h"
#include "comps/light.h"

entt::entity factories::createTree(
	const std::shared_ptr<entt::registry>& registry,
	const std::shared_ptr<ModelManager>& modelMngr,
	glm::vec3 pos, glm::vec3 scale
) {
	auto tree = registry->create();

	registry->emplace<comps::position>(tree, pos);
	registry->emplace<comps::orientation>(tree);
	registry->emplace<comps::scale>(tree, scale);
	registry->emplace<comps::transform>(tree);

	modelMngr->CreateInstance(tree, "tree");

	return tree;
}

entt::entity factories::createTemple(
	const std::shared_ptr<entt::registry>& registry,
	const std::shared_ptr<ModelManager>& modelMngr,
	glm::vec3 pos, glm::vec3 rot, glm::vec3 scale
) {
	auto temple = registry->create();

	registry->emplace<comps::position>(temple, pos);

	glm::quat y = glm::angleAxis(glm::radians(rot.x), VEC_UP);
	glm::quat x = glm::angleAxis(glm::radians(rot.y), VEC_RIGHT);
	glm::quat z = glm::angleAxis(glm::radians(rot.z), VEC_FORWARD);
	registry->emplace<comps::orientation>(temple, z * x * y);

	registry->emplace<comps::scale>(temple, scale);
	registry->emplace<comps::transform>(temple);

	modelMngr->CreateInstance(temple, "temple");

	return temple;
}

entt::entity factories::createSphere(
	const std::shared_ptr<entt::registry>& registry,
	const std::shared_ptr<ModelManager>& modelMngr,
	glm::vec3 pos, glm::vec3 rot, glm::vec3 scale,
	Color::RGB color
) {
	auto sphere = registry->create();

	registry->emplace<comps::position>(sphere, pos);

	glm::quat y = glm::angleAxis(glm::radians(rot.x), VEC_UP);
	glm::quat x = glm::angleAxis(glm::radians(rot.y), VEC_RIGHT);
	glm::quat z = glm::angleAxis(glm::radians(rot.z), VEC_FORWARD);
	registry->emplace<comps::orientation>(sphere, z * x * y);

	registry->emplace<comps::scale>(sphere, scale);
	registry->emplace<comps::transform>(sphere);

	modelMngr->CreateInstance(sphere, "sphere");

	return sphere;
}

entt::entity factories::createDirLight(
	const std::shared_ptr<entt::registry>& registry,
	const Color::RGB& color,
	float ambient, float diffuse, float specular,
	float yaw, float pitch, float roll
) {
	auto light = registry->create();


	registry->emplace<comps::lightEmitter>(light, color, ambient, diffuse, specular);
	registry->emplace<comps::dirLight>(light);

	glm::quat y = glm::angleAxis(glm::radians(yaw), VEC_UP);
	glm::quat x = glm::angleAxis(glm::radians(pitch), VEC_RIGHT);
	glm::quat z = glm::angleAxis(glm::radians(roll), VEC_FORWARD);
	registry->emplace<comps::orientation>(light, z * y * x);

	return light;
}