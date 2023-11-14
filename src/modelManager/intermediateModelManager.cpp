#include "intermediateModelManager.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>


// tyniobj::index_t hashing function
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

// ModelManager implementation

IntermediateModelManager::IntermediateModelManager() {

}

IntermediateModelManager::~IntermediateModelManager() {

}

void IntermediateModelManager::loadMesh(Object& target, const meshId_t& meshId, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
	Mesh mesh{};

	std::unordered_map<tinyobj::index_t, uint16_t, index_t_hash, index_t_equal> indexTracker;
	uint16_t numVertices = 0;

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
				mesh.indices.push_back(indexTracker.at(idx));
				continue;
			}

			// Create vertex data
			Vertex vertex{};

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
			mesh.vertices.push_back(vertex);

			// update indices and index tracker
			indexTracker.emplace(idx, numVertices);
			mesh.indices.push_back(numVertices);
			numVertices++;
		}
		index_offset += fv;
	}
	
	target.meshes.emplace(meshId, std::move(mesh));
}


const Object& IntermediateModelManager::loadObject(const objectId_t& objectId) {
	const std::filesystem::path objectPath = getPathFromId(objectId);
	
	tinyobj::ObjReaderConfig reader_config;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(objectPath.string(), reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();

	Object object{};
	
	for (const auto& shape : shapes) {
		loadMesh(object, meshId_t{ shape.name }, attrib, shape);
	}

	objects.emplace(objectId, object);

	return objects.at(objectId);
}

const Object& IntermediateModelManager::getOrLoadObject(const objectId_t& objectId){
	if (objects.find(objectId) != objects.end()) {
		return objects.at(objectId);
	}

	// load object
	return loadObject(objectId);
}

// Material

Material IntermediateModelManager::loadColorMaterial(const rapidjson::GenericObject<false, rapidjson::Value>& data) {
	Material material{ MaterialType::Color };
	
	assert(data.HasMember("diffuse"));
	assert(data["diffuse"].IsString());
	material.color.diffuse = Color(data["diffuse"].GetString());

	assert(data.HasMember("specular"));
	assert(data["specular"].IsString());
	material.color.specular = Color(data["specular"].GetString());

	assert(data.HasMember("shininess"));
	assert(data["shininess"].IsFloat());
	material.color.shininess = data["shininess"].GetFloat();

	return material;
}

const Material& IntermediateModelManager::loadMaterial(const materialId_t& materialId) {
	const std::filesystem::path materialPath = getPathFromId(materialId);
	
	std::ifstream file{ materialPath };

	if (!file.is_open()) {
		std::stringstream err;
		err << "Error opening material " << materialPath << ".";
		throw std::runtime_error(err.str());
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	rapidjson::Document document{};
	document.Parse(buffer.str().c_str());

	assert(document.HasMember("type"));
	assert(document["type"].IsString());
	std::string type = document["type"].GetString();

	assert(document.HasMember("data"));
	assert(document["data"].IsObject());
	auto data = document["data"].GetObject();

	Material material{};

	if (type == "color") {
		material = loadColorMaterial(data);
	}
	// TODO: add other material types

	materials.emplace(materialId, material);
	
	return materials.at(materialId);
}

const Material& IntermediateModelManager::getOrLoadMaterial(const materialId_t& materialId) {
	if (materials.find(materialId) != materials.end()) {
		return materials.at(materialId);
	}

	// load material
	return loadMaterial(materialId);
}

Model IntermediateModelManager::LoadModel(const modelId_t& modelId) {
	std::filesystem::path path = getPathFromId(modelId);
	
	std::ifstream file{ path };
	
	if (!file.is_open()) {
		std::stringstream err;
		err << "Error opening model " << path << ".";
		throw std::runtime_error(err.str());
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();

	file.close();

	rapidjson::Document document{};
	document.Parse(buffer.str().c_str());

	// parse the data
	assert(document.IsObject());

	assert(document.HasMember("object"));
	assert(document["object"].IsString());
	objectId_t objectId = document["object"].GetString();

	// load the object
	Object object = getOrLoadObject(objectId);

	// parse the materials
	assert(document.HasMember("material"));
	assert(document["material"].IsObject());
	auto jsonMaterials = document["material"].GetObject();

	Model model{};

	model.objectId = objectId;

	for (auto& member : jsonMaterials) {
		meshId_t meshId{ member.name.GetString() };
		materialId_t materialId = jsonMaterials[meshId.str.c_str()].GetString();

		if (object.meshes.find(meshId) == object.meshes.end()) {
			throw std::runtime_error("Every mesh specified in the model has to be part of the object.");
		}

		Material material = getOrLoadMaterial(materialId);

		model.materialPerMesh.emplace(meshId, materialId);
	}

	return model;
}

const Object& IntermediateModelManager::GetObject(const objectId_t& objectId) {
	assert(objects.find(objectId) != objects.end());
	return objects.at(objectId);
}

const Material& IntermediateModelManager::GetMaterial(const materialId_t& materialId) {
	assert(materials.find(materialId) != materials.end());
	return materials.at(materialId);
}
