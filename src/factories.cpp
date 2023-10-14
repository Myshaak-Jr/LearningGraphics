#include "factories.h"

#include <sstream>

#include "comps/position.h"
#include "comps/orientation.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/rotatedByKeyboard.h"
#include "comps/color.h"
#include "comps/dirLight.h"

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

	auto trunk = offspring->at("trunk");
	registry->emplace<comps::material>(trunk, C_BROWN);
	auto lowerCone = offspring->at("lower-cone");
	registry->emplace<comps::material>(lowerCone, C_TREE_GREEN);
	auto upperCone = offspring->at("upper-cone");
	registry->emplace<comps::material>(upperCone, C_TREE_GREEN);

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
	registry->emplace<comps::orientation>(temple, rot.x, rot.y, rot.z);
	
	registry->emplace<comps::scale>(temple, scale);
	registry->emplace<comps::transform>(temple);

	auto offspring = modelMngr->GenEntities("temple", temple, registry);
	
	for (const auto& child : *offspring) {
		registry->emplace<comps::shaderProgram>(child.second, prgMngr->getShaderProgram("diffuse"));
	}

	auto platform = offspring->at("platform");
	registry->emplace<comps::material>(platform, C_STONE2);

	for (int i = 1; i <= 6; i++) {
		std::stringstream columnName;
		std::stringstream columnTopName;

		columnName << "column-" << i;
		columnTopName << "column-top-" << i;

		auto column = offspring->at(columnName.str());
		registry->emplace<comps::material>(column, C_STONE1);
		auto columnTop = offspring->at(columnTopName.str());
		registry->emplace<comps::material>(columnTop, C_STONE1);
	}

	return temple;
}

entt::entity factories::createDirLight(const std::unique_ptr<entt::registry>& registry, const color::RGB& color, const glm::vec3& rot) {
	auto light = registry->create();

	registry->emplace<comps::dirLight>(light, color);
	registry->emplace<comps::orientation>(light, rot.x, rot.y, rot.z);

	return light;
}