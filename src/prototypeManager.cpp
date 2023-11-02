#include "prototypeManager.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include <rapidjson/document.h>


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

std::string ModelManager::getNameFromPath(const std::string& pathStr) {
	std::filesystem::path path(pathStr);
	return path.stem().string();
}



std::unique_ptr<Mesh> ModelManager::loadMesh(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

	std::unordered_map<tinyobj::index_t, GLushort, index_t_hash, index_t_equal> indexTracker;
	GLushort numVertices = 0;

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
				mesh->indices.push_back(indexTracker.at(idx));
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
			mesh->vertices.push_back(vertex);

			// update indices and index tracker
			indexTracker.emplace(idx, numVertices);
			mesh->indices.push_back(numVertices);
			numVertices++;
		}
		index_offset += fv;
	}
	
	return std::move(mesh);
}


std::vector<std::string> ModelManager::loadObject(const std::string& objectPath, const objectId_t& objectId) {
	tinyobj::ObjReaderConfig reader_config;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(objectPath, reader_config)) {
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

	std::vector<meshId_t> meshIds;

	for (const auto& shape : shapes) {
		std::unique_ptr<Mesh> mesh = loadMesh(attrib, shape);

		std::stringstream meshId;
		meshId << objectId << "@" << shape.name;
	
		meshes.emplace(meshId.str(), std::move(mesh));
		meshIds.push_back(meshId.str());
	}

	objects.emplace(objectId, meshIds);

	return meshIds;
}

std::vector<std::string> ModelManager::getOrLoadObject(const std::string& objectPath) {
	// convert the path to the relative format, so as to use it as the key of the objects
	const std::string objectsBasePath = "./models/objects/";

	objectId_t objectId;
	try {
		objectId = std::filesystem::relative(objectPath, objectsBasePath).string();
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	// object already loaded return
	if (objects.find(objectId) != objects.end()) return objects.at(objectId);
	

	// load object
	return loadObject(objectPath, objectId);
}


void ModelManager::LoadModel(const std::string& path) {
	namespace json = rapidjson;

	std::ifstream file{ path };
	
	if (!file.is_open()) {
		std::stringstream ss;
		ss << "Error opening model path " << path << ".";
		throw std::runtime_error(ss.str());
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();

	file.close();

	json::Document document{};
	document.Parse(buffer.str().c_str());

	// parse the data
	assert(document.IsObject());

	assert(document.HasMember("model"));
	assert(document["model"].IsString());
	std::string modelPath = document["model"].GetString();

	// create the model


	//

	std::unordered_map<std::string, std::string> materialPaths;

	assert(document.HasMember("material"));
	assert(document["material"].IsObject());

	json::GenericObject jsonMaterials = document["material"].GetObject();

	for (auto& member : jsonMaterials) {
		std::string modelPart = member.name.GetString();
		std::string materialPath = jsonMaterials[modelPart.c_str()].GetString();

		materialPaths.emplace(modelPart, materialPath);
	}


}