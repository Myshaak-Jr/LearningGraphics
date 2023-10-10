#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>

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

class IMesh {
protected:
	static size_t combine_hash(size_t lhs, size_t rhs);

	struct index_t_hash {
		size_t operator()(const tinyobj::index_t& index) const;
	};

	struct index_t_equal {
		bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const;
	};
public:
	virtual comps::mesh exportAsComponent() const = 0;
	virtual ~IMesh() {}
};

template <class TIndex>
class Mesh : public IMesh {
	static_assert(std::is_integral<TIndex>::value, "TIndex must be an integral type");
	static_assert(std::is_unsigned<TIndex>::value, "TIndex must be an unsigned type");
	static_assert(!std::is_same<TIndex, uint64_t>::value, "TIndex must not be uint64_t");

private:
	std::vector<Vertex> vertices;
	std::vector<TIndex> indices;

	std::string name;

	GLenum getIndexType() const;

public:
	Mesh(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);
	Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices);
	~Mesh() = default;

	comps::mesh exportAsComponent() const override;
};

class Model {
private:
	std::unordered_map<std::string, std::unique_ptr<IMesh>> meshes;
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

	template <class TIndex>
	void addMesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices);
};

class ModelManager {
private:
	// Private members
	std::unordered_map<std::string, Model> models;

	// Private methods
	void assertExisting(const std::string& name) const;
	void assertNotExisting(const std::string& name) const;

public:
	ModelManager() = default;
	~ModelManager() = default;

	void LoadModel(const std::string& name, const char* filename, const char* materialPath, bool triangulate = true);
	void AddModel(const std::string& name);
	
	std::unique_ptr<std::unordered_map<std::string, entt::entity>> GenEntities(const std::string& name, entt::entity, const std::unique_ptr<entt::registry>& registry);

	template <class TIndex>
	void AddMeshToModel(const std::string& modelName, const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices);
};


template <class TIndex>
Mesh<TIndex>::Mesh(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
	name = shape.name;
	std::cout << "Loading mesh '" << name << "'" << std::endl;

	std::unordered_map<tinyobj::index_t, TIndex, index_t_hash, index_t_equal> indexTracker;
	TIndex numVertices = 0;

	// for each face
	size_t index_offset = 0;
	for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
		size_t fv = size_t(shape.mesh.num_face_vertices[f]);

		// Loop over vertices in the face
		for (size_t v = 0; v < fv; v++) {
			// access to vertex
			tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

			// if the idx is already in the index tracker use the already generated vertex
			if (indexTracker.find(idx) != indexTracker.end()) {
				// update indices
				indices.push_back(indexTracker.at(idx));
				continue;
			}

			// Create vertex data
			Vertex vertex;

			vertex.vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
			vertex.vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
			vertex.vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

			// Check if `normal_index` is zero or positive. negative = no normal data
			if (idx.normal_index >= 0) {
				vertex.nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
				vertex.ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
				vertex.nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
			}
			else {
				vertex.nx = 0.0f;
				vertex.ny = 0.0f;
				vertex.nz = 0.0f;
			}

			// Check if `texcoord_index` is zero or positive. negative = no texcoord data
			if (idx.texcoord_index >= 0) {
				vertex.tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
				vertex.ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
			}
			else {
				vertex.tx = 0.0f;
				vertex.ty = 0.0f;
			}

			// update vertices
			vertices.push_back(vertex);

			// update indices and index tracker
			indexTracker.emplace(idx, numVertices);
			indices.push_back(numVertices);
			numVertices++;
		}
		index_offset += fv;
	}
}


template <class TIndex>
Mesh<TIndex>::Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices)
	: name(name)
	, vertices(vertices)
	, indices(indices)
{}

template <class TIndex>
comps::mesh Mesh<TIndex>::exportAsComponent() const {
	comps::mesh mesh = {};

	GLenum err;
	while (glGetError() != GL_NO_ERROR);

	// Generate GL objects
	glGenVertexArrays(1, &mesh.vao);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "glGenVertexArrays: OpenGL error: " << err << std::endl;
	}

	glGenBuffers(1, &mesh.vbo);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "glGenBuffers: OpenGL error: " << err << std::endl;
	}
	glGenBuffers(1, &mesh.ebo);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "glGenBuffers: OpenGL error: " << err << std::endl;
	}

	// Bind GL objects
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);

	// Buffer data
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(TIndex), indices.data(), GL_STATIC_DRAW);
	mesh.elementCount = (GLsizei)indices.size();

	// Setup Pointers
	//   positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vx));
	//   normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
	//   texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tx));

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}

	mesh.indexType = getIndexType();

	return mesh;
}

template <class TIndex>
void Model::addMesh(const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices) {
	if (meshes.find(meshName) != meshes.end()) {
		std::stringstream ss;
		ss << "Mesh with name " << meshName << " already exists in model " << name << "." << std::endl;
		throw std::runtime_error(ss.str());
	}

	meshes.insert(std::make_pair(meshName, std::make_unique<Mesh<TIndex>>(meshName, vertices, indices)));

	// Generate OpenGL objects for this mesh
	genMeshComp(meshName);
}


template <class TIndex>
void ModelManager::AddMeshToModel(
	const std::string& modelName,
	const std::string& meshName,
	const std::vector<Vertex>& vertices,
	const std::vector<TIndex>& indices
) {
	assertExisting(modelName);

	models.at(modelName).addMesh(meshName, vertices, indices);
}