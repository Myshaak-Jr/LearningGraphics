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
#include <string>

struct id_t {
	std::string str;

	id_t(const std::string str) : str(str) {}
	id_t(const char* str) : str(str) {}
	id_t() : str("") {}
};

bool operator==(const id_t& lhs, const id_t& rhs) {
	return lhs.str == rhs.str;
}

namespace std {
	template <>
	struct hash<id_t> {
		size_t operator()(const id_t& s) const {
			return hash<std::string>{}(s.str);
		}
	};
}

struct shaderId_t : id_t {
	shaderId_t(const std::string str) : id_t(str) {}
	shaderId_t(const char* str) : id_t(str) {}
	shaderId_t() : id_t("") {}
};
struct materialId_t : id_t {
	materialId_t(const std::string str) : id_t(str) {}
	materialId_t(const char* str) : id_t(str) {}
	materialId_t() : id_t("") {}
};
struct meshId_t : id_t {
	meshId_t(const std::string str) : id_t(str) {}
	meshId_t(const char* str) : id_t(str) {}
	meshId_t() : id_t("") {}
};
struct objectId_t : id_t {
	objectId_t(const std::string str) : id_t(str) {}
	objectId_t(const char* str) : id_t(str) {}
	objectId_t() : id_t("") {}
};
struct modelId_t : id_t {
	modelId_t(const std::string str) : id_t(str) {}
	modelId_t(const char* str) : id_t(str) {}
	modelId_t() : id_t("") {}
};


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
