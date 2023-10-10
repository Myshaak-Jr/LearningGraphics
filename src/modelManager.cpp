#include "modelManager.h"

#include <iostream>
#include <sstream>

#include "comps/child.h"
#include "comps/position.h"
#include "comps/rotation.h"
#include "comps/scale.h"
#include "comps/transform.h"



size_t combine_hash(size_t lhs, size_t rhs) {
	if constexpr (sizeof(size_t) >= 8) {
		lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
	}
	else {
		lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
	}
	return lhs;
}

struct index_t_hash {
	size_t operator()(const tinyobj::index_t& index) const {
		size_t vertexHash = std::hash<int>{}(index.vertex_index);
		size_t normalHash = std::hash<int>{}(index.normal_index);
		size_t texcoordHash = std::hash<int>{}(index.texcoord_index);
		return combine_hash(combine_hash(vertexHash, normalHash), texcoordHash);
	}
};

struct index_t_equal {
	bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const {
		return (
			a.vertex_index == b.vertex_index &&
			a.normal_index == b.normal_index &&
			a.texcoord_index == b.texcoord_index
		);
	}
};

Mesh::Mesh(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
	name = shape.name;
	std::cout << "Loading mesh '" << name << "'" << std::endl;

	std::unordered_map<tinyobj::index_t, GLushort, index_t_hash, index_t_equal> indexTracker;
	Uint32 numVertices = 0;

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

	//std::cout << "vertex data: " << std::endl;
	//for (const auto& vertex : vertices) {
	//	std::cout << "\t" << vertex.vx << ", " << vertex.vy << ", " << vertex.vz << std::endl;
	//}

	//std::cout << "index data: " << std::endl;
	//int i = 0;
	//for (const auto& index : indices) {
	//	std::cout << index;
	//	if ((++i) % 3 == 0) {
	//		std::cout << "\n";
	//	}
	//	else {
	//		std::cout << ", ";
	//	}
	//}
}

Mesh::Mesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<GLushort>& indices)
	: name(name)
	, vertices(vertices)
	, indices(indices)
{}

comps::mesh Mesh::exportAsComponent() const {
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
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

	return mesh;
}

Model::Model(
	const std::string& name,
	const tinyobj::attrib_t& attrib,
	const std::vector<tinyobj::shape_t>& shapes,
	const std::vector<tinyobj::material_t>& materials
)
	: name(name)
{
	// load meshes
	for (const auto& shape : shapes) {
		meshes.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(shape.name),
			std::forward_as_tuple(attrib, shape)
		);
	}

	// TODO: load materials
}

Model::Model(const std::string& name) : name(name) {}

Model::~Model() {
	for (auto& mesh : meshComps) {
		glDeleteVertexArrays(1, &mesh.second.vao);
		GLuint buffers[2] = { mesh.second.vbo, mesh.second.ebo };
		glDeleteBuffers(2, buffers);
	}
}

void Model::genAllMeshComps() {
	meshComps.clear();
	
	for (const auto& mesh : meshes) {
		meshComps.emplace(mesh.first, mesh.second.exportAsComponent());
	}
}

void Model::genMeshComp(const std::string& name) {
	assert(meshes.find(name) != meshes.end());

	meshComps.emplace(name, meshes.at(name).exportAsComponent());
}

std::unique_ptr<std::unordered_map<std::string, entt::entity>> Model::generateEntities(entt::entity root, const std::unique_ptr<entt::registry>& registry) {
	auto entities = std::make_unique<std::unordered_map<std::string, entt::entity>>();
	
	for (const auto& mesh : meshComps) {
		entt::entity entity = registry->create();

		registry->emplace<comps::child>(entity, root);
		registry->emplace<comps::position>(entity);
		registry->emplace<comps::rotation>(entity);
		registry->emplace<comps::scale>(entity);
		registry->emplace<comps::transform>(entity);
		registry->emplace<comps::mesh>(entity, mesh.second);

		entities->emplace(mesh.first, entity);
	}

	return std::move(entities);
}

void Model::addMesh(const std::string& meshName, const std::vector<Vertex>& vertices, const std::vector<GLushort>& indices) {
	if (meshes.find(meshName) != meshes.end()) {
		std::stringstream ss;
		ss << "Mesh with name " << meshName << " already exists in model " << name << "." << std::endl;
		throw std::runtime_error(ss.str());
	}

	meshes.emplace(std::piecewise_construct, std::forward_as_tuple(meshName), std::forward_as_tuple(meshName, vertices, indices));
}

void ModelManager::LoadModel(const std::string& name, const char* filename, const char* materialPath, bool triangulate ) {
	assertNotExisting(name);

	std::cout << "Loading model '" << filename << "'" << std::endl;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, materialPath, triangulate);

	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << "ERR: " << err << std::endl;
	}

	if (!ret) {
		throw std::runtime_error("Failed to load/parse .obj.\n");
	}

	models.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple(attrib, shapes, materials)
	);

	genAllMeshComps(name);
}

void ModelManager::AddModel(const std::string& name) {
	assertNotExisting(name);

	models.emplace(name, name);
}

void ModelManager::assertExisting(const std::string& name) const {
	if (models.find(name) != models.end()) return;
	
	std::stringstream ss;
	ss << "Model " << name << " not found.";
	throw std::runtime_error(ss.str());
}

void ModelManager::assertNotExisting(const std::string& name) const {
	if (models.find(name) == models.end()) return;
	
	std::stringstream ss;
	ss << "Model " << name << " already exists.";
	throw std::runtime_error(ss.str());
}

void ModelManager::genAllMeshComps(const std::string& name) {
	assertExisting(name);
	
	models.at(name).genAllMeshComps();
}

void ModelManager::genMeshComp(const std::string& model, const std::string& mesh) {
	assertExisting(model);

	models.at(model).genMeshComp(mesh);
}

std::unique_ptr<std::unordered_map<std::string, entt::entity>>
ModelManager::GenEntities(
	const std::string& name, entt::entity root, const std::unique_ptr<entt::registry>& registry)
{
	assertExisting(name);

	return std::move(models.at(name).generateEntities(root, registry));
}

void ModelManager::AddMeshToModel(
	const std::string& modelName,
	const std::string& meshName,
	const std::vector<Vertex>& vertices,
	const std::vector<GLushort>& indices
) {
	assertExisting(modelName);

	models.at(modelName).addMesh(meshName, vertices, indices);
}