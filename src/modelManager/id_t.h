/*
The ID of each object is the same as the path from the base directory without the file extension:
	models base directory: models/
	materials base directory: models/materials/
	objects base directory: models/objects/

	mesh ID is special because it is the mesh name specified in the .obj

	In the .json files the values shall be id, NOT paths.
*/

#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>

#include <rapidjson/document.h>


struct shaderId_t {
	std::string str;

	shaderId_t(const std::string& str) : str(str) {}
	shaderId_t(const char* str) : str(str) {}
	shaderId_t() : str("") {}
};
struct materialId_t {
	std::string str;

	materialId_t(const std::string& str) : str(str) {}
	materialId_t(const char* str) : str(str) {}
	materialId_t() : str("") {}
};
struct meshId_t {
	std::string str;

	meshId_t(const std::string& str) : str(str) {}
	meshId_t(const char* str) : str(str) {}
	meshId_t() : str("") {}
};
struct objectId_t {
	std::string str;

	objectId_t(const std::string& str) : str(str) {}
	objectId_t(const char* str) : str(str) {}
	objectId_t() : str("") {}
};
struct modelId_t {
	std::string str;

	modelId_t(const std::string& str) : str(str) {}
	modelId_t(const char* str) : str(str) {}
	modelId_t() : str("") {}
};

struct uniqueMeshId_t {
	std::string str;
	objectId_t objectId;
	meshId_t meshId;

	uniqueMeshId_t(const objectId_t& objectId, const meshId_t& meshId) : str(objectId.str + "/" + meshId.str), objectId(objectId), meshId(meshId) {}
	uniqueMeshId_t() : str("") {}
};


// unordered map
template <class T_Id>
struct CompareId {
	bool operator()(const T_Id& id1, const T_Id& id2) const noexcept {
		return id1.str == id2.str;
	}
};

template <class T_Id>
struct HashId {
	size_t operator()(const T_Id& id) const noexcept {
		return std::hash<std::string>{}(id.str);
	}
};

template <class T_Id, class T_Val>
using id_umap = std::unordered_map<T_Id, T_Val, HashId<T_Id>, CompareId<T_Id>>;


// file handling
template <class T_Id> struct fileParamethers { static std::filesystem::path directory; static std::string extension; };

template <class T_Id>
static T_Id getIdFromPath(const std::filesystem::path& path) {
	// example path: "./models/materials/tree/leaves.json" -> id: "tree/leaves"

	auto relative = std::filesystem::relative(path, fileParamethers<T_Id>::directory);

	std::stringstream id;
	for (auto path_iter = relative.begin(); path_iter != std::prev(path.end()); ++path_iter) {
		if (!id.str().empty()) {
			id << "/";
		}
		id << path_iter->string();
	}

	id << path.stem();

	return T_Id{ id.str() };
}

template <class T_Id>
std::filesystem::path getPathFromId(const T_Id& id) {
	std::filesystem::path relative = id.str + fileParamethers<T_Id>::extension;
	return fileParamethers<T_Id>::directory / relative;
}

template <class T_Id>
rapidjson::Document parseJsonFile(const T_Id& id) {
	std::filesystem::path jsonPath = getPathFromId(id);

	// read the file
	std::ifstream file{ jsonPath };

	if (!file.is_open()) {
		std::stringstream err;
		err << "Error opening json file: " << jsonPath << ".";
		throw std::runtime_error(err.str());
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	file.close();

	// create the document
	rapidjson::Document document{};
	document.Parse(buffer.str().c_str());

	return document;
}