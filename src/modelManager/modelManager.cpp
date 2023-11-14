#include "modelManager.h"


GLModelManager::GLModelManager(std::shared_ptr<entt::registry> registry) : registry(registry) {
	intermediate = std::make_unique<IntermediateModelManager>();
}

GLModelManager::~GLModelManager() {}


void GLModelManager::CreateInstance(const Model& model) {

	for (const auto& [meshId, materialId] : model.materialPerMesh) {
		entt::entity entity = registry->create();

		emplaceMesh(entity, { model.objectId, meshId });
		emplaceMaterial(entity, materialId);
	}
}

void GLModelManager::emplaceMesh(entt::entity entity, const uniqueMeshId_t& meshId) {
	const comps::mesh& mesh = getOrCreateMesh(meshId);
	registry->emplace<comps::mesh>(entity, mesh);
}

const comps::mesh& GLModelManager::getOrCreateMesh(const uniqueMeshId_t& meshId) {
	if (meshes.find(meshId) != meshes.end()) {
		return meshes.at(meshId);
	}

	return createMesh(meshId);
}

const comps::mesh& GLModelManager::createMesh(const uniqueMeshId_t& meshId) {
	const Mesh& originalMesh = intermediate->GetObject(meshId.first).meshes.at(meshId.second);


}
