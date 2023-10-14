#include "factories.h"

#include <sstream>

#include "comps/position.h"
#include "comps/orientation.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/rotatedByKeyboard.h"
#include "comps/material.h"
#include "comps/light.h"

entt::entity factories::createTree(
	const std::unique_ptr<entt::registry>& registry,
	const std::unique_ptr<ModelManager>& modelMngr,
	const std::unique_ptr<ProgramManager>& prgMngr,
	glm::vec3 pos, glm::vec3 scale
) {
	auto tree = registry->create();

	registry->emplace<comps::position>(tree, pos);
	registry->emplace<comps::orientation>(tree);
	registry->emplace<comps::scale>(tree, scale);
	registry->emplace<comps::transform>(tree);

	auto offspring = modelMngr->GenEntities("tree", tree, registry);

	for (const auto& child : *offspring) {
		registry->emplace<comps::shaderProgram>(child.second, prgMngr->getShaderProgram("diffuse"));
	}

	return tree;
}

entt::entity factories::createTemple(
	const std::unique_ptr<entt::registry>& registry,
	const std::unique_ptr<ModelManager>& modelMngr,
	const std::unique_ptr<ProgramManager>& prgMngr,
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

	auto offspring = modelMngr->GenEntities("temple", temple, registry);
	
	for (const auto& child : *offspring) {
		registry->emplace<comps::shaderProgram>(child.second, prgMngr->getShaderProgram("diffuse"));
	}

	return temple;
}

entt::entity factories::createDirLight(
	const std::unique_ptr<entt::registry>& registry,
	const myColor::RGB& color,
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