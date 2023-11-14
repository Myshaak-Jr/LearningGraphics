#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <memory>
#include <filesystem>

#include <tinyobj/tiny_obj_loader.h>
#include <rapidjson/document.h>

#include "model.h"


class IntermediateModelManager {
private:
	// std::unordered_map<shaderId_t, Shader> shaders;
	std::unordered_map<materialId_t, Material> materials;
	// the first string is the relative path of the .obj file from the ./model/objects/
	std::unordered_map<objectId_t, Object> objects;

	const Object& loadObject(const objectId_t& objectId);
	const Object& getOrLoadObject(const objectId_t& objectId);

	static Material loadColorMaterial(const rapidjson::GenericObject<false, rapidjson::Value>& data);

	const Material& loadMaterial(const materialId_t& materialId);
	const Material& getOrLoadMaterial(const materialId_t& materialId);

	static void loadMesh(Object& target, const meshId_t& meshId, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);

public:
	IntermediateModelManager();
	~IntermediateModelManager();

	Model LoadModel(const modelId_t& modelId);

	const Object& GetObject(const objectId_t& objectId);
	const Material& GetMaterial(const materialId_t& materialId);
};