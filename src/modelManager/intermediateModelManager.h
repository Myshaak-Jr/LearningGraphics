/*
	Loads JSON and .OBJ files into my id based representation
*/

#pragma once

#include <unordered_set>
#include <string>
#include <memory>
#include <filesystem>

#include <tinyobj/tiny_obj_loader.h>
#include <rapidjson/document.h>

#include "model.h"


class IntermediateModelManager {
private:
	id_umap<shaderId_t, Shader> shaders;
	id_umap<materialId_t, Material> materials;
	id_umap<objectId_t, Object> objects;

	void loadObject(const objectId_t& objectId);
	void ensureObjectLoaded(const objectId_t& objectId);
	[[nodiscard]] const Object& getOrLoadObject(const objectId_t& objectId);

	[[nodiscard]] static Material loadColorMaterial(const rapidjson::GenericObject<false, rapidjson::Value>& data);

	void loadMaterial(const materialId_t& materialId);
	void ensureMaterialLoaded(const materialId_t& materialId);
	[[nodiscard]] const Material& getOrLoadMaterial(const materialId_t& materialId);

	void loadShader(const shaderId_t& shaderId);
	void ensureShaderLoaded(const shaderId_t& shaderId);
	[[nodiscard]] const Shader& getOrLoadShader(const shaderId_t& shaderId);

	static void loadMesh(Object& target, const meshId_t& meshId, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);

	const Object& parseModelObject(Model& model, const rapidjson::Document& document);
	void parseModelMaterial(Model& model, const rapidjson::Document& document, const Object& object);
	void parseModelShader(Model& model, const rapidjson::Document& document, const Object& object);

public:
	IntermediateModelManager();
	~IntermediateModelManager();

	[[nodiscard]] Model LoadModel(const modelId_t& modelId);

	[[nodiscard]] const Object& GetObject(const objectId_t& objectId);
	[[nodiscard]] const Material& GetMaterial(const materialId_t& materialId);
	[[nodiscard]] const Shader& GetShader(const shaderId_t& shaderId);
};