#include "intermediateModelManager.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "../hashHelper.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*      MODEL MANAGER                                                                  */
/////////////////////////////////////////////////////////////////////////////////////////

IntermediateModelManager::IntermediateModelManager() {}

IntermediateModelManager::~IntermediateModelManager() {}


/////////////////////////////////////////////////////////////////////////////////////////
/*      OBJECT & MESH                                                                  */
/////////////////////////////////////////////////////////////////////////////////////////

void IntermediateModelManager::loadMesh(Object& target, const meshId_t& meshId, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape) {
	Mesh mesh{};

	std::unordered_map<tinyobj::index_t, uint16_t, HashIndex, CompareIndex> indexTracker;
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

void IntermediateModelManager::loadObject(const objectId_t& objectId) {
	const std::filesystem::path objectPath = getPathFromId(objectId);
	
	tinyobj::ObjReaderConfig reader_config;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(objectPath.string(), reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();

	Object object{};
	
	for (const auto& shape : shapes) {
		loadMesh(object, meshId_t{ shape.name }, attrib, shape);
	}

	objects.emplace(objectId, object);
}

void IntermediateModelManager::ensureObjectLoaded(const objectId_t& objectId) {
	if (objects.find(objectId) == objects.end())
		loadObject(objectId);
}

const Object& IntermediateModelManager::getOrLoadObject(const objectId_t& objectId){
	ensureObjectLoaded(objectId);
	return objects.at(objectId);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*      MATERIAL                                                                       */
/////////////////////////////////////////////////////////////////////////////////////////

Material IntermediateModelManager::loadColorMaterial(const rapidjson::GenericObject<false, rapidjson::Value>& data) {
	Material material{ MaterialType::Color };
	
	assert(data.HasMember("diffuse"));
	assert(data["diffuse"].IsString());
	material.color.diffuse = Color(data["diffuse"].GetString());

	assert(data.HasMember("specular"));
	assert(data["specular"].IsString());
	material.color.specular = Color(data["specular"].GetString());

	assert(data.HasMember("shininess"));
	assert(data["shininess"].IsNumber());
	material.color.shininess = data["shininess"].GetFloat();

	return material;
}

void IntermediateModelManager::loadMaterial(const materialId_t& materialId) {
	rapidjson::Document document = parseJsonFile(materialId);
	
	// parse the data
	assert(document.IsObject());

	assert(document.HasMember("type"));
	assert(document["type"].IsString());
	std::string type = document["type"].GetString();

	assert(document.HasMember("data"));
	assert(document["data"].IsObject());
	auto data = document["data"].GetObject();

	Material material{};

	// load material data
	if (type == "color") {
		material = loadColorMaterial(data);
	}
	// TODO: add other material types

	materials.emplace(materialId, material);
}

void IntermediateModelManager::ensureMaterialLoaded(const materialId_t& materialId) {
	if (materials.find(materialId) == materials.end())
		loadMaterial(materialId);
}

const Material& IntermediateModelManager::getOrLoadMaterial(const materialId_t& materialId) {
	ensureMaterialLoaded(materialId);
	return materials.at(materialId);
}

/////////////////////////////////////////////////////////////////////////////////////////
/*      SHADER                                                                         */
/////////////////////////////////////////////////////////////////////////////////////////

void IntermediateModelManager::loadShader(const shaderId_t& shaderId) {
	rapidjson::Document document = parseJsonFile(shaderId);

	// parse the data
	assert(document.IsObject());

	bool hasVertex = document.HasMember("vertex") && document["vertex"].IsString();
	bool hasGeometry = document.HasMember("geometry") && document["geometry"].IsString();
	bool hasFragment = document.HasMember("fragment") && document["fragment"].IsString();

	Shader shader{};

	std::filesystem::path basePath = fileParamethers<shaderId_t>::directory;
	
	shader.vertex = (hasVertex) ? basePath / document["vertex"].GetString() : "";
	shader.geometry = (hasGeometry) ? basePath / document["geometry"].GetString() : "";
	shader.fragment = (hasFragment) ? basePath / document["fragment"].GetString() : "";

	shaders.emplace(shaderId, shader);
}

void IntermediateModelManager::ensureShaderLoaded(const shaderId_t& shaderId) {
	if (shaders.find(shaderId) == shaders.end())
		loadShader(shaderId);
}

const Shader& IntermediateModelManager::getOrLoadShader(const shaderId_t& shaderId) {
	ensureShaderLoaded(shaderId);
	return shaders.at(shaderId);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*      MODEL                                                                          */
/////////////////////////////////////////////////////////////////////////////////////////

const Object& IntermediateModelManager::parseModelObject(Model& model, const rapidjson::Document& document) {
	assert(document.HasMember("object"));
	assert(document["object"].IsString());
	objectId_t objectId = document["object"].GetString();

	model.objectId = objectId;

	// load the object
	return getOrLoadObject(objectId);
}

void IntermediateModelManager::parseModelMaterial(Model& model, const rapidjson::Document& document, const Object& object) {
	assert(document.HasMember("material"));

	if (document["material"].IsString()) {
		materialId_t materialId = document["material"].GetString();
		ensureMaterialLoaded(materialId);
		
		for (const auto& mesh : object.meshes) {
			meshId_t meshId = mesh.first;
			model.materialPerMesh.emplace(meshId, materialId);
		}

		return;
	}
	
	if (document["material"].IsObject()) {
		auto materialIds = document["material"].GetObject();
		
		for (const auto& mesh : object.meshes) {
			meshId_t meshId = mesh.first;
			const char* meshIdCString = meshId.str.c_str();

			assert(materialIds.HasMember(meshIdCString));
			assert(materialIds[meshIdCString].IsString());

			materialId_t materialId = materialIds[meshIdCString].GetString();

			ensureMaterialLoaded(materialId);
			model.materialPerMesh.emplace(mesh.first, materialId);
		}
		
		return;
	}
	
	throw std::runtime_error("Model has to have at least one material specified.");
}

void IntermediateModelManager::parseModelShader(Model& model, const rapidjson::Document& document, const Object& object) {
	assert(document.HasMember("shader"));

	if (document["shader"].IsString()) {
		shaderId_t shaderId = document["shader"].GetString();
		ensureShaderLoaded(shaderId);

		for (const auto& mesh : object.meshes) {
			meshId_t meshId = mesh.first;
			model.shaderPerMesh.emplace(meshId, shaderId);
		}

		return;
	}

	if (document["shader"].IsObject()) {
		auto shaderIds = document["shader"].GetObject();

		for (const auto& mesh : object.meshes) {
			meshId_t meshId = mesh.first;
			const char* meshIdCString = meshId.str.c_str();

			assert(shaderIds.HasMember(meshIdCString));
			assert(shaderIds[meshIdCString].IsString());

			shaderId_t shaderId = shaderIds[meshIdCString].GetString();

			ensureShaderLoaded(shaderId);
			model.shaderPerMesh.emplace(meshId, shaderId);
		}

		return;
	}

	throw std::runtime_error("Model has to have at least one material specified.");
}

Model IntermediateModelManager::LoadModel(const modelId_t& modelId) {
	rapidjson::Document document = parseJsonFile(modelId);

	assert(document.IsObject());


	Model model{};
	const Object& object = parseModelObject(model, document);
	parseModelMaterial(model, document, object);
	parseModelShader(model, document, object);

	return model;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*      GETTERS                                                                        */
/////////////////////////////////////////////////////////////////////////////////////////

const Object& IntermediateModelManager::GetObject(const objectId_t& objectId) {
	assert(objects.find(objectId) != objects.end());
	return objects.at(objectId);
}

const Material& IntermediateModelManager::GetMaterial(const materialId_t& materialId) {
	assert(materials.find(materialId) != materials.end());
	return materials.at(materialId);
}

const Shader& IntermediateModelManager::GetShader(const shaderId_t& shaderId) {
	assert(shaders.find(shaderId) != shaders.end());
	return shaders.at(shaderId);
}