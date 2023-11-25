#pragma once

#include "glModelManager.h"


class ModelManager {
private:
	std::shared_ptr<IntermediateModelManager> intermediateMngr;
	std::shared_ptr<GLModelManager> glMngr;

	id_umap<modelId_t, Model> models;

public:
	ModelManager(std::shared_ptr<entt::registry> registry);
	~ModelManager();

	void LoadModel(const modelId_t& modelId);
	void CreateInstance(entt::entity parent, const modelId_t& modelId);

	const id_umap<shaderId_t, comps::shader>& GetShaders() const;
};