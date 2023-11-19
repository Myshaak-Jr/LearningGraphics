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
	std::shared_ptr<IntermediateModelManager> intermediateMngr;

	id_umap<uniqueMeshId_t, comps::mesh> meshes;
	id_umap<shaderId_t, comps::shader> shaders;

	void emplaceShader(entt::entity entity, const shaderId_t& shaderId);
	void emplaceMesh(entt::entity entity, const uniqueMeshId_t& meshId);
	void emplaceMaterial(entt::entity entity, const materialId_t& materialId);

	void emplaceColorMaterial(entt::entity entity, const ColorData& colorData);
	void emplaceTextureMaterial(entt::entity entity, const TextureData& textureData);

	static GLuint compileShader(const std::filesystem::path& path, GLenum shaderType);
	void linkShader(comps::shader& shader, const Shader& originalShader);
	void createShader(const shaderId_t& shaderId);
	void ensureShaderCreated(const shaderId_t& shaderId);
	const comps::shader& getOrCreateShader(const shaderId_t& shaderId);

	void createMesh(const uniqueMeshId_t& meshId);
	void ensureMeshCreated(const uniqueMeshId_t& meshId);
	const comps::mesh& getOrCreateMesh(const uniqueMeshId_t& meshId);

public:
	GLModelManager(std::shared_ptr<entt::registry> registry, std::shared_ptr<IntermediateModelManager> intermediateMngr);
	~GLModelManager();

	void CreateInstance(entt::entity parent, const Model& model);

	void PrepareModel(const Model& model);

	const id_umap<shaderId_t, comps::shader>& GetShaders() const;
};
