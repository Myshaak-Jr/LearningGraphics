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


class ModelManager {
private:
	// ModelManager sub-classes
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

		comps::mesh exportAsComponent() const;
	};

	class Model {
	private:
		std::unordered_map<std::string, Mesh> meshes;
		std::unordered_map<std::string, comps::mesh> meshComps;

	public:
		Model(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials);
		~Model();

		void generateGLMeshes();

		/* 
		 * Returns a map of all the other entities, where the key is the name of the mesh
		 */
		std::unique_ptr<std::unordered_map<std::string, entt::entity>> generateEntities(entt::entity root, const std::unique_ptr<entt::registry>& registry);
	};

	// Private members
	std::unordered_map<std::string, Model> models;

	// Private methods
	void assertExisting(const std::string& name) const;
	void genMeshComps(const std::string& name);

public:
	ModelManager() = default;
	~ModelManager() = default;

	void LoadModel(const std::string& name, const char* filename, const char* materialPath, bool triangulate = true);
	
	std::unique_ptr<std::unordered_map<std::string, entt::entity>> GenEntities(const std::string& name, entt::entity, const std::unique_ptr<entt::registry>& registry);
};
