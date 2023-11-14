#pragma once

#include <vector>
#include <unordered_map>

#include "../color.h"
#include "id_t.h"

struct Shader {
	// TODO:
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
	std::unordered_map<meshId_t, Mesh> meshes;
};

struct Model {
	objectId_t objectId;
	std::unordered_map<meshId_t, materialId_t> materialPerMesh;
};
