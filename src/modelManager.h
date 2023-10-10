#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

#include <SDL2/SDL_stdinc.h>
#include <glad/glad.h>
#include <entt/entt.hpp>
#include <tinyobj/tiny_obj_loader.h>

#include "comps/mesh.h"


struct Vertex {
	float vx, vy, vz;
	float nx, ny, nz;
	float tx, ty;
};

class Mesh {
private:
	std::vector<Vertex> vertices;
	std::vector<GLushort> indices;

	std::string name;

public:
	Mesh(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);
	Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<GLushort>& indices);

	comps::mesh exportAsComponent() const;
};

class Model {
private:
	std::unordered_map<std::string, Mesh> meshes;
	std::unordered_map<std::string, comps::mesh> meshComps;

	std::string name;

public:
	Model(const std::string& name, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials);
	Model(const std::string& name);
	~Model();

	void genAllMeshComps();
	void genMeshComp(const std::string& name);

	/*
	 * Returns a map of all the other entities, where the key is the name of the mesh
	 */
	std::unique_ptr<std::unordered_map<std::string, entt::entity>> generateEntities(entt::entity root, const std::unique_ptr<entt::registry>& registry);

	void addMesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<GLushort>& indices);
};

class ModelManager {
private:
	// Private members
	std::unordered_map<std::string, Model> models;

	// Private methods
	void assertExisting(const std::string& name) const;
	void assertNotExisting(const std::string& name) const;
	void genAllMeshComps(const std::string& name);
	void genMeshComp(const std::string& model, const std::string& mesh);

public:
	ModelManager() = default;
	~ModelManager() = default;

	void LoadModel(const std::string& name, const char* filename, const char* materialPath, bool triangulate = true);
	void AddModel(const std::string& name);
	
	std::unique_ptr<std::unordered_map<std::string, entt::entity>> GenEntities(const std::string& name, entt::entity, const std::unique_ptr<entt::registry>& registry);

	void AddMeshToModel(const std::string& modelName, const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<GLushort>& indices);
};
