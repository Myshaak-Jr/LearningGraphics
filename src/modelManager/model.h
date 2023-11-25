#pragma once

#include <vector>
#include <unordered_map>
#include <filesystem>

#include "../color.h"
#include "id_t.h"

struct Shader {
	std::filesystem::path vertex;
	std::filesystem::path geometry;
	std::filesystem::path fragment;
};

enum class MaterialType {
	Color, Texture
};

struct ColorData {
	Color diffuse;
	Color specular;
	float shininess;
};

struct TextureData {
	// TODO:
};

struct Material {
	MaterialType type;

	union {
		ColorData color;
		TextureData texture;
	};

	shaderId_t shaderId;
};

struct Vertex {
	float vx, vy, vz;
	float nx, ny, nz;
	float tx, ty;
};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

struct Object {
	id_umap<meshId_t, Mesh> meshes;
};

struct Model {
	objectId_t objectId;
	id_umap<meshId_t, materialId_t> materialPerMesh;
	id_umap<meshId_t, shaderId_t> shaderPerMesh;
};
