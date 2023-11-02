#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>

#include "comps/mesh.h"
#include "comps/material.h"
#include "comps/shader.h"

#include <tinyobj/tiny_obj_loader.h>


using shaderId_t = std::string;
using materialId_t = std::string;
using meshId_t = std::string;
using objectId_t = std::string;

struct Shader {
	//comps::shader shaderPrototype;
};



class IMaterial {

public:
	virtual ~IMaterial() = default;
};

class ColorMaterial : public IMaterial {
private:
	Color diffuse;
	Color specular;
	float shininess;

	shaderId_t shaderId;

	//comps::colorMaterial materialPrototype;

public:
	~ColorMaterial() override = default;

	
};

/* Vertex data containing vertex position, vertex normal and texture uv coordinates */
struct Vertex {
	float vx, vy, vz;
	float nx, ny, nz;
	float tx, ty;
};



struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<GLushort> indices;

	//comps::mesh meshPrototype;
};

struct Model {
	std::unordered_map<meshId_t, materialId_t> meshes;
};

class ModelManager {
private:
	std::unordered_map<shaderId_t, std::unique_ptr<Shader>> shaders;

	std::unordered_map<materialId_t, std::unique_ptr<IMaterial>> materials;

	std::unordered_map<meshId_t, std::unique_ptr<Mesh>> meshes;
	// first string is the relative path of the .obj file from the ./model/objects/ and the vector is a vector of meshIds
	std::unordered_map<objectId_t, std::vector<meshId_t>> objects;

	std::unordered_map<std::string, Model> models;


	static std::string getNameFromPath(const std::string& pathStr);

	std::vector<meshId_t> loadObject(const std::string& objectPath, const objectId_t& objectId);
	std::vector<meshId_t> getOrLoadObject(const std::string& objectPath);

	static std::unique_ptr<Mesh> loadMesh(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);

public:
	ModelManager();
	~ModelManager();

	void LoadModel(const std::string& path);
};

//
//class IMesh {
//protected:
//	static size_t combine_hash(size_t lhs, size_t rhs);
//
//	struct index_t_hash {
//		size_t operator()(const tinyobj::index_t& index) const;
//	};
//
//	struct index_t_equal {
//		bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const;
//	};
//public:
//	virtual std::pair<comps::mesh, comps::material> exportComponents() const = 0;
//	virtual ~IMesh() {}
//};
//
//template <class TIndex>
//class Mesh : public IMesh {
//	static_assert(std::is_integral<TIndex>::value, "TIndex must be an integral type");
//	static_assert(std::is_unsigned<TIndex>::value, "TIndex must be an unsigned type");
//	static_assert(!std::is_same<TIndex, uint64_t>::value, "TIndex must not be uint64_t");
//
//private:
//	std::vector<Vertex> vertices;
//	std::vector<TIndex> indices;
//	comps::material material;
//	
//	std::string name;
//
//	GLenum getIndexType() const;
//
//	void loadVerticesAndIndices(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);
//	void loadMaterial(const std::vector<tinyobj::material_t>& materials, const tinyobj::shape_t& shape);
//
//public:
//	Mesh(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::material_t>& materials, const tinyobj::shape_t& shape);
//	Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices, const comps::material& material);
//	~Mesh() = default;
//
//	std::pair<comps::mesh, comps::material> exportComponents() const override;
//
//	const std::string& getName() const;
//};
//
//class Model {
//private:
//	std::unordered_map<std::string, std::unique_ptr<IMesh>> meshes;
//	std::unordered_map<std::string, std::pair<comps::mesh, comps::material>> meshComps;
//
//	std::string name;
//
//public:
//	Model(const std::string& name, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials);
//	Model(const std::string& name);
//	~Model();
//
//	void genAllMeshComps();
//	void genMeshComp(const std::string& name);
//
//	/*
//	 * Returns a map of all the other entities, where the key is the name of the mesh
//	 */
//	std::unique_ptr<std::unordered_map<std::string, entt::entity>> generateEntities(entt::entity root, const std::shared_ptr<entt::registry>& registry);
//
//	template <class TIndex>
//	void addMesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices, const comps::material& material);
//
//	template <class TIndex>
//	void addMesh(const Mesh<TIndex>& mesh);
//	
//	template <class TIndex>
//	void addMesh(std::unique_ptr<Mesh<TIndex>> mesh);
//};
//
///* thread-safe model manager */
//class ModelManager {
//private:
//	// Private members
//	std::unordered_map<std::string, Model> models;
//	std::mutex mtx;
//
//	// Private methods
//	void assertExisting(const std::string& name) const;
//	void assertNotExisting(const std::string& name) const;
//
//public:
//	ModelManager() = default;
//	~ModelManager() = default;
//
//	void LoadModel(const std::string& name, const char* filename, const char* materialPath, bool triangulate = true);
//	void AddModel(const std::string& name);
//	
//	/* This function is using non-thread-safe registry and should be called with caution */
//	std::unique_ptr<std::unordered_map<std::string, entt::entity>> GenEntities(const std::string& name, entt::entity root, const std::shared_ptr<entt::registry>& registry);
//
//	template <class TIndex>
//	void AddMeshToModel(const std::string& modelName, const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices, const comps::material& material);
//
//	template <class TIndex>
//	void AddMeshToModel(const std::string& modelName, const Mesh<TIndex>& mesh);
//
//	template <class TIndex>
//	void AddMeshToModel(const std::string& modelName, std::unique_ptr<Mesh<TIndex>> mesh);
//};
//
//
//template <class TIndex>
//void Mesh<TIndex>::loadVerticesAndIndices(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
//
//	std::unordered_map<tinyobj::index_t, TIndex, index_t_hash, index_t_equal> indexTracker;
//	TIndex numVertices = 0;
//
//	// for each face
//	size_t index_offset = 0;
//	for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
//		size_t fv = size_t(shape.mesh.num_face_vertices[f]);
//
//		// Loop over vertices in the face
//		for (size_t v = 0; v < fv; v++) {
//			// access to vertex
//			tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
//
//			// if the idx is already in the index tracker use the already generated vertex
//			if (indexTracker.find(idx) != indexTracker.end()) {
//				// update indices
//				indices.push_back(indexTracker.at(idx));
//				continue;
//			}
//
//			// Create vertex data
//			Vertex vertex;
//
//			vertex.vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
//			vertex.vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
//			vertex.vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
//
//			// Check if `normal_index` is zero or positive. negative = no normal data
//			if (idx.normal_index >= 0) {
//				vertex.nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
//				vertex.ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
//				vertex.nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
//			}
//			else {
//				vertex.nx = 0.0f;
//				vertex.ny = 0.0f;
//				vertex.nz = 0.0f;
//			}
//
//			// Check if `texcoord_index` is zero or positive. negative = no texcoord data
//			if (idx.texcoord_index >= 0) {
//				vertex.tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
//				vertex.ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
//			}
//			else {
//				vertex.tx = 0.0f;
//				vertex.ty = 0.0f;
//			}
//
//			// update vertices
//			vertices.push_back(vertex);
//
//			// update indices and index tracker
//			indexTracker.emplace(idx, numVertices);
//			indices.push_back(numVertices);
//			numVertices++;
//		}
//		index_offset += fv;
//	}
//}
//
//template <class TIndex>
//void Mesh<TIndex>::loadMaterial(const std::vector<tinyobj::material_t>& materials, const tinyobj::shape_t& shape) {
//	if (shape.mesh.material_ids.empty()) {
//		std::cout << "Warning: Mesh " << name << " has no material, using default white one.";
//		material = comps::material(myColor::RGB(1.0f), myColor::RGB(0.8f), myColor::RGB(0.5f), 255.0f);
//		return;
//	}
//
//	int material_id = shape.mesh.material_ids[0];
//	const tinyobj::material_t& mat = materials[material_id];
//	
//	myColor::RGB ambient{ mat.ambient[0], mat.ambient[1] , mat.ambient[2] };
//	myColor::RGB diffuse{ mat.diffuse[0], mat.diffuse[1] , mat.diffuse[2] };
//	myColor::RGB specular{ mat.specular[0], mat.specular[1] , mat.specular[2] };
//	float shininess = mat.shininess;
//
//	material = comps::material(ambient, diffuse, specular, shininess);
//}
//
//template <class TIndex>
//Mesh<TIndex>::Mesh(const tinyobj::attrib_t& attrib, const std::vector<tinyobj::material_t>& materials, const tinyobj::shape_t& shape) {
//	name = shape.name;
//	std::cout << "Loading mesh '" << name << "'" << std::endl;
//
//	loadVerticesAndIndices(attrib, shape);
//	loadMaterial(materials, shape);
//}
//
//
//template <class TIndex>
//Mesh<TIndex>::Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices, const comps::material& material)
//	: name(name)
//	, vertices(vertices)
//	, indices(indices)
//	, material(material)
//{}
//
//template <class TIndex>
//std::pair<comps::mesh, comps::material> Mesh<TIndex>::exportComponents() const {
//	comps::mesh mesh = {};
//
//	GLenum err;
//	while (glGetError() != GL_NO_ERROR);
//
//	// Generate GL objects
//	glGenVertexArrays(1, &mesh.vao);
//	while ((err = glGetError()) != GL_NO_ERROR) {
//		std::cerr << "glGenVertexArrays: OpenGL error: " << err << std::endl;
//	}
//
//	glGenBuffers(1, &mesh.vbo);
//	while ((err = glGetError()) != GL_NO_ERROR) {
//		std::cerr << "glGenBuffers: OpenGL error: " << err << std::endl;
//	}
//	glGenBuffers(1, &mesh.ebo);
//	while ((err = glGetError()) != GL_NO_ERROR) {
//		std::cerr << "glGenBuffers: OpenGL error: " << err << std::endl;
//	}
//
//	// Bind GL objects
//	glBindVertexArray(mesh.vao);
//	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
//
//	// Buffer data
//	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(TIndex), indices.data(), GL_STATIC_DRAW);
//	mesh.elementCount = (GLsizei)indices.size();
//
//	// Setup Pointers
//	//   positions
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vx));
//	//   normals
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
//	//   texture coordinates
//	glEnableVertexAttribArray(2);
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tx));
//
//	// Unbind
//	glBindVertexArray(0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//
//	while ((err = glGetError()) != GL_NO_ERROR) {
//		std::cerr << "OpenGL error: " << err << std::endl;
//	}
//
//	mesh.indexType = getIndexType();
//
//	return std::make_pair(mesh, material);
//}
//
//template <class TIndex>
//const std::string& Mesh<TIndex>::getName() const {
//	return name;
//}
//
//
//template <class TIndex>
//void Model::addMesh(const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<TIndex>& indices, const comps::material& material) {
//	if (meshes.find(meshName) != meshes.end()) {
//		std::stringstream ss;
//		ss << "Mesh with name " << meshName << " already exists in model " << name << "." << std::endl;
//		throw std::runtime_error(ss.str());
//	}
//
//	meshes.insert(std::make_pair(meshName, std::make_unique<Mesh<TIndex>>(meshName, vertices, indices, material)));
//
//	// Generate OpenGL objects for this mesh
//	genMeshComp(meshName);
//}
//
//template <class TIndex>
//void Model::addMesh(const Mesh<TIndex>& mesh) {
//	const std::string& meshName = mesh.getName();
//
//	if (meshes.find(meshName) != meshes.end()) {
//		std::stringstream ss;
//		ss << "Mesh with name " << meshName << " already exists in model " << name << "." << std::endl;
//		throw std::runtime_error(ss.str());
//	}
//
//	meshes.insert(std::make_pair(meshName, std::make_unique<Mesh<TIndex>>(mesh)));
//
//	// Generate OpenGL objects for this mesh
//	genMeshComp(meshName);
//}
//
//template <class TIndex>
//void Model::addMesh(std::unique_ptr<Mesh<TIndex>> mesh) {
//	const std::string& meshName = mesh->getName();
//
//	if (meshes.find(meshName) != meshes.end()) {
//		std::stringstream ss;
//		ss << "Mesh with name " << meshName << " already exists in model " << name << "." << std::endl;
//		throw std::runtime_error(ss.str());
//	}
//
//	meshes.insert(std::make_pair(meshName, std::move(mesh)));
//
//	// Generate OpenGL objects for this mesh
//	genMeshComp(meshName);
//}
//
//template <class TIndex>
//void ModelManager::AddMeshToModel(
//	const std::string& modelName,
//	const std::string& meshName,
//	const std::vector<Vertex>& vertices,
//	const std::vector<TIndex>& indices,
//	const comps::material& material
//) {
//	assertExisting(modelName);
//
//	{
//		std::unique_lock<std::mutex> lock(mtx);
//		models.at(modelName).addMesh(meshName, vertices, indices, material);
//	}
//
//	std::osyncstream(std::cout) << "Added mesh " << meshName << " to model " << modelName << std::endl;
//}
//
//template <class TIndex>
//void ModelManager::AddMeshToModel(const std::string& modelName, const Mesh<TIndex>& mesh) {
//	assertExisting(modelName);
//
//	std::string meshName = mesh.getName();
//
//	{
//		std::unique_lock<std::mutex> lock(mtx);
//		models.at(modelName).addMesh(mesh);
//	}
//
//	std::osyncstream(std::cout) << "Added mesh " << meshName << " to model " << modelName << std::endl;
//}
//
//template <class TIndex>
//void ModelManager::AddMeshToModel(const std::string& modelName, std::unique_ptr<Mesh<TIndex>> mesh) {
//	assertExisting(modelName);
//
//	std::string meshName = mesh->getName();
//
//	{
//		std::unique_lock<std::mutex> lock(mtx);
//		models.at(modelName).addMesh(std::move(mesh));
//	}
//
//	std::osyncstream(std::cout) << "Added mesh " << meshName << " to model " << modelName << std::endl;
//}