#pragma once

#include "perlin.h"
#include "modelManager.h"


class Terrain {
private:
	float width;
	float height;
	uint32_t res_x;
	uint32_t res_y;

	Perlin perlin;

	void createVertex(std::vector<Vertex>& vertices, uint32_t x, uint32_t y);
	void calcIndices(std::vector<uint32_t>& indices, uint32_t x, uint32_t y);

public:
	Terrain(float width, float height, uint32_t resolution, float scale);
	~Terrain() = default;
	
	/* creates a mesh from the perlin noise, with origin being at the center */
	void ExportAsModel(const std::string& name, const std::unique_ptr<ModelManager>& modelMngr);
	//void GetHeight(float x, float y);
};