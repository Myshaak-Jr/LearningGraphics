#include "terrain.h"


Terrain::Terrain(float width, float height, uint32_t resolution, float scale)
	: width(width)
	, height(height)
	, res_x(resolution)
	, res_y((uint32_t)ceil(resolution * width/height))
	, perlin(scale, std::random_device{}())
{}

void Terrain::createVertex(std::vector<Vertex>& vertices, uint32_t x, uint32_t y) {
	glm::vec2 pos(x, y);
	pos /= glm::vec2(res_x - 1, res_y - 1);
	pos -= 0.5f;
	pos *= glm::vec2(width, height);
	
	Vertex vertex = { 0.0f };

	glm::vec3 vertexPos = glm::vec3(pos.x, perlin.get(pos), pos.y);
	vertex.vx = vertexPos.x;
	vertex.vy = vertexPos.y;
	vertex.vz = vertexPos.z;

	glm::vec3 normal = perlin.getNormal(pos);
	vertex.nx = normal.x;
	vertex.ny = normal.y;
	vertex.nz = normal.z;

	vertices.push_back(vertex);
}

void Terrain::calcIndices(std::vector<uint32_t>& indices, uint32_t x, uint32_t y) {
	if (x == res_x - 1 || y == res_y - 1) return;

	uint32_t cornerIdxs[4] = {
		y*res_x + x,
		y*res_x + (x + 1),
		(y + 1)*res_x + (x + 1),
		(y + 1)*res_x + x
	};

	indices.push_back(cornerIdxs[0]);
	indices.push_back(cornerIdxs[1]);
	indices.push_back(cornerIdxs[2]);

	indices.push_back(cornerIdxs[2]);
	indices.push_back(cornerIdxs[3]);
	indices.push_back(cornerIdxs[0]);
}

void Terrain::ExportAsModel(const std::string& name, const std::unique_ptr<ModelManager>& modelMngr) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	
	// create vertices
	for (uint32_t y = 0; y < res_y; y++) {
		for (uint32_t x = 0; x < res_x; x++) {
			createVertex(vertices, x, y);
			calcIndices(indices, x, y);
		}
	}

	modelMngr->AddModel(name);
	modelMngr->AddMeshToModel(name, "ground", vertices, indices);
}