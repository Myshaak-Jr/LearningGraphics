/*
	A class for converting and loading my Model, Object, Material, ... structs into OpenGL's objects.
*/

#pragma once

#include <memory>

#include <entt/entt.hpp>

#include "comps/material.h"
#include "comps/mesh.h"
#include "comps/shader.h"

#include "intermediateModelManager.h"

class GLModelManager {
private:
	std::shared_ptr<entt::registry> registry;
	std::unique_ptr<IntermediateModelManager> intermediate;

	using uniqueMeshId_t = std::pair<objectId_t, meshId_t>;

	std::unordered_map<uniqueMeshId_t, comps::mesh> meshes;

	void emplaceMesh(entt::entity entity, const uniqueMeshId_t& meshId);
	void emplaceMaterial(entt::entity entity, const materialId_t& materialId);

	const comps::mesh& getOrCreateMesh(const uniqueMeshId_t& meshId);
	const comps::mesh& createMesh(const uniqueMeshId_t& meshId);

public:
	GLModelManager(std::shared_ptr<entt::registry> registry);
	~GLModelManager();

	void CreateInstance(const Model& model);
};
