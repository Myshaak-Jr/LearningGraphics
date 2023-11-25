#include "modelManager.h"


ModelManager::ModelManager(std::shared_ptr<entt::registry> registry) {
	intermediateMngr = std::make_unique<IntermediateModelManager>();
	glMngr = std::make_unique<GLModelManager>(registry, intermediateMngr);
}

ModelManager::~ModelManager() {}


void ModelManager::LoadModel(const modelId_t& modelId) {
	models.emplace(modelId, intermediateMngr->LoadModel(modelId));
	glMngr->PrepareModel(models.at(modelId));
}

void ModelManager::CreateInstance(entt::entity parent, const modelId_t& modelId) {
	if (models.find(modelId) == models.end()) {
		std::stringstream ss;
		ss << "Model " << modelId.str << " is not loaded.";
		throw std::runtime_error(ss.str());
	}

	glMngr->CreateInstance(parent, models.at(modelId));
}

const id_umap<shaderId_t, comps::shader>& ModelManager::GetShaders() const {
	return glMngr->GetShaders();
}
