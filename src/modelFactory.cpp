#include "modelManager.h"

#include <syncstream>

#include "comps/child.h"
#include "comps/position.h"
#include "comps/orientation.h"
#include "comps/scale.h"
#include "comps/transform.h"


size_t IMesh::combine_hash(size_t lhs, size_t rhs) {
	if constexpr (sizeof(size_t) >= 8) {
		lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
	}
	else {
		lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
	}
	return lhs;
}

size_t IMesh::index_t_hash::operator()(const tinyobj::index_t& index) const {
	size_t vertexHash = std::hash<int>{}(index.vertex_index);
	size_t normalHash = std::hash<int>{}(index.normal_index);
	size_t texcoordHash = std::hash<int>{}(index.texcoord_index);
	return combine_hash(combine_hash(vertexHash, normalHash), texcoordHash);
}

bool IMesh::index_t_equal::operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const {
	return (
		a.vertex_index == b.vertex_index &&
		a.normal_index == b.normal_index &&
		a.texcoord_index == b.texcoord_index
	);
}

GLenum Mesh<uint8_t>::getIndexType() const {
	return GL_UNSIGNED_BYTE;
}

GLenum Mesh<uint16_t>::getIndexType() const {
	return GL_UNSIGNED_SHORT;
}

GLenum Mesh<uint32_t>::getIndexType() const {
	return GL_UNSIGNED_INT;
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
		meshes.insert(std::make_pair(shape.name, std::make_unique<Mesh<GLushort>>(attrib, materials, shape)));
	}

	// TODO: load materials

	// generate OpenGL objects
	genAllMeshComps();
}

Model::Model(const std::string& name) : name(name) {}

Model::~Model() {
	for (auto& mesh : meshComps) {
		glDeleteVertexArrays(1, &mesh.second.first.vao);
		GLuint buffers[2] = { mesh.second.first.vbo, mesh.second.first.ebo };
		glDeleteBuffers(2, buffers);
	}
}

void Model::genAllMeshComps() {
	meshComps.clear();
	
	for (const auto& mesh : meshes) {
		meshComps.emplace(mesh.first, mesh.second->exportComponents());
	}
}

void Model::genMeshComp(const std::string& name) {
	assert(meshes.find(name) != meshes.end());

	meshComps.emplace(name, meshes.at(name)->exportComponents());
}

std::unique_ptr<std::unordered_map<std::string, entt::entity>> Model::generateEntities(entt::entity root, const std::shared_ptr<entt::registry>& registry) {
	auto entities = std::make_unique<std::unordered_map<std::string, entt::entity>>();
	
	for (const auto& mesh : meshComps) {
		entt::entity entity;

		entity = registry->create();

		registry->emplace<comps::child>(entity, root);
		registry->emplace<comps::position>(entity);
		registry->emplace<comps::orientation>(entity);
		registry->emplace<comps::scale>(entity);
		registry->emplace<comps::transform>(entity);
		registry->emplace<comps::mesh>(entity, mesh.second.first);
		registry->emplace<comps::material>(entity, mesh.second.second);

		entities->emplace(mesh.first, entity);
	}

	return std::move(entities);
}

void ModelManager::LoadModel(const std::string& name, const char* filename, const char* materialPath, bool triangulate ) {
	assertNotExisting(name);

	std::osyncstream(std::cout) << "Loading model '" << filename << "'" << std::endl;

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

	{
		std::unique_lock lock(mtx);
		models.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(name),
			std::forward_as_tuple(name, attrib, shapes, materials)
		);
	}
}

void ModelManager::AddModel(const std::string& name) {
	assertNotExisting(name);

	{
		std::unique_lock lock(mtx);
		models.emplace(name, name);
	}

	std::osyncstream(std::cout) << "Added model " << name << std::endl;
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

std::unique_ptr<std::unordered_map<std::string, entt::entity>>
ModelManager::GenEntities(const std::string& name, entt::entity root, const std::shared_ptr<entt::registry>& registry) {
	assertExisting(name);

	// NOT SURE WHETHER THERE SHOULD OR SHOULDN'T BE MUTEX LOCK HERE!

	return std::move(models.at(name).generateEntities(root, registry));
}
