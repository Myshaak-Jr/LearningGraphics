#include "terrain.h"

#include <syncstream>


myTerrain::Terrain::Terrain(
	const std::string& name, std::shared_ptr<ModelManager>& modelMngr, std::mutex& modelMngrMtx,
	std::shared_ptr<entt::registry>& registry, std::mutex& registryMtx,
	const std::vector<TerrainType>& terrainTypes,
	int chunkSize, float scale, int octaves, float persistance, float lacunarity, uint32_t seed
)
	: name(name)
	, modelMngr(modelMngr)
	, modelMngrMtx(modelMngrMtx)
	, registry(registry)
	, registryMtx(registryMtx)
	, chunkWidth(chunkSize)
	, chunkHeight(chunkSize)
	, scale(scale)
	, octaves(octaves)
	, persistance(persistance)
	, lacunarity(lacunarity)
	, terrainTypes(terrainTypes)
	, seed(seed)
{
	std::mt19937 rng{ seed };
	std::uniform_int_distribution<uint32_t> dist{};

	for (int i = 0; i < octaves; i++) {
		perlins.push_back(noiseType(dist(rng)));
	}

	unsigned int numChunkLoaders = std::thread::hardware_concurrency();

	for (size_t i = 0; i < numChunkLoaders; ++i) {
		std::thread thread{ chunkLoaderJob, this };
		chunkLoaders.emplace_back(thread);
	}
}

myTerrain::Terrain::~Terrain() {
	{
		std::unique_lock<std::mutex> lock(queueMtx);
		stopChunkLoaders = true;
	}
	cv.notify_all();
	for (std::thread& chunkLoader : chunkLoaders) {
		chunkLoader.join();
	}
}

void myTerrain::Terrain::LoadChunks(const std::vector<glm::ivec2>& chunks) {
	for (const auto& chunk : chunks) {
		LoadChunk(chunk);
	}
}

void myTerrain::Terrain::LoadChunk(glm::ivec2 chunkPos) {
	{
		std::unique_lock<std::mutex> lock(queueMtx);
		chunksToLoad.push(chunkPos);
	}
}

void myTerrain::Terrain::chunkLoaderJob() {
	while (true) {
		glm::ivec2 chunkPos;

		{
			// create the lock
			std::unique_lock<std::mutex> lock(queueMtx);

			// wait until an order arrives
			cv.wait(lock, [this]() { return stopChunkLoaders || !chunksToLoad.empty(); });

			// return if stop was called
			if (stopChunkLoaders) return;

			// get the chunk to load
			chunkPos = chunksToLoad.front();
			chunksToLoad.pop();
		}

		// load chunk
		loadChunk(chunkPos);
	}
}

void myTerrain::Terrain::loadChunk(glm::ivec2 chunkPos) {
	std::osyncstream(std::cout) << "Loading chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;

	auto heightMap = generateHeightMap(chunkPos);
	generateMesh(heightMap);

	std::osyncstream(std::cout) << "Loaded chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;
}


std::vector<std::vector<float>> myTerrain::Terrain::generateHeightMap(glm::ivec2 chunkPos) {
	std::vector<std::vector<float>> noiseMap(chunkHeight, std::vector<float>(chunkWidth, 0.0f));

	std::vector<myMath::Perlin> perlins;

	std::mt19937 rng{ seed };
	std::uniform_int_distribution<uint32_t> dist{};

	for (int i = 0; i < octaves; i++) {
		perlins.push_back(myMath::Perlin(dist(rng)));
	}

	if (scale <= 0.0f) {
		scale = 0.0001f;
	}

	float maxNoiseHeight = 0.0f;

	for (int y = 0; y < chunkHeight; y++) {
		for (int x = 0; x < chunkWidth; x++) {
			float amplitude = 1;
			float frequency = 1;
			float noiseHeight = 0;

			float sampleX = static_cast<float>(x + chunkPos.x) / scale * frequency;
			float sampleY = static_cast<float>(y + chunkPos.y) / scale * frequency;

			for (int i = 0; i < octaves; i++) {
				float value = perlins[i].get(sampleX, sampleY);
				noiseHeight += value * amplitude;
				maxNoiseHeight += amplitude;

				amplitude *= persistance;
				frequency *= lacunarity;
			}

			noiseMap[y][x] = noiseHeight;
		}
	}

	for (int y = 0; y < chunkHeight; y++) {
		for (int x = 0; x < chunkWidth; x++) {
			noiseMap[y][x] /= maxNoiseHeight;
		}
	}

	return noiseMap;
}

void myTerrain::Terrain::createVertex(std::vector<Vertex>& vertices, uint32_t x, uint32_t y) {

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

//void Terrain::calcIndices(std::vector<uint32_t>& indices, uint32_t x, uint32_t y) {
//	if (x == res_x - 1 || y == res_y - 1) return;
//
//	uint32_t cornerIdxs[4] = {
//		y*res_x + x,
//		y*res_x + (x + 1),
//		(y + 1)*res_x + (x + 1),
//		(y + 1)*res_x + x
//	};
//
//	indices.push_back(cornerIdxs[0]);
//	indices.push_back(cornerIdxs[1]);
//	indices.push_back(cornerIdxs[2]);
//
//	indices.push_back(cornerIdxs[2]);
//	indices.push_back(cornerIdxs[3]);
//	indices.push_back(cornerIdxs[0]);
//}

//void Terrain::ExportAsModel(const std::string& name, const std::unique_ptr<ModelManager>& modelMngr) {
//	std::vector<Vertex> vertices;
//	std::vector<uint32_t> indices;
//	
//	// create vertices
//	for (uint32_t y = 0; y < res_y; y++) {
//		for (uint32_t x = 0; x < res_x; x++) {
//			createVertex(vertices, x, y);
//			calcIndices(indices, x, y);
//		}
//	}
//
//	modelMngr->AddModel(name);
//	modelMngr->AddMeshToModel(name, "ground", vertices, indices);
//}

//struct TerrainType {
//	std::string name;
//	float height;
//	color::RGB color;
//
//	TerrainType(std::string name, float height, color::RGB color) : name(name), height(height), color(color) {}
//};

std::vector<std::vector<C_RGB>> generateColorMap(const std::vector<std::vector<float>>& heightMap, const std::vector<TerrainType>& regions) {
	size_t map_w = heightMap[0].size();
	size_t map_h = heightMap.size();

	std::vector<std::vector<C_RGB>> colorMap(map_h, std::vector<C_RGB>(map_w, C_RGB(0.0f)));

	for (int y = 0; y < map_h; y++) {
		for (int x = 0; x < map_w; x++) {
			for (const auto& region : regions) {
				if (heightMap[y][x] <= region.height) {
					colorMap[y][x] = region.color;

					break;
				}
			}
		}
	}

	return colorMap;
}

bool renderToTexture(std::vector<std::vector<C_RGB>> colorMap, Simulation* sim, SDL_Renderer* renderer, int pixelWidth) {
	// Create / Recreate the texture if needed
	if (sim->mapTexture == NULL) {
		sim->mapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, sim->width * pixelWidth, sim->height * pixelWidth);
		if (sim->mapTexture == NULL) {
			Log_Error("Failed to create SDL_Texture.");
			return false;
		}
	}
	else {
		int w, h;
		SDL_QueryTexture(sim->mapTexture, NULL, NULL, &w, &h);

		if (w != sim->width * pixelWidth || h != sim->height * pixelWidth) {
			SDL_DestroyTexture(sim->mapTexture);
			sim->mapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, sim->width * pixelWidth, sim->height * pixelWidth);
			if (sim->mapTexture == NULL) {
				Log_Error("Failed to create SDL_Texture.");
				return false;
			}
		}
	}

	SDL_SetRenderTarget(renderer, sim->mapTexture);

	for (int y = 0; y < sim->height; y++) {
		for (int x = 0; x < sim->width; x++) {
			SDL_Rect rect = {
				x * pixelWidth,
				y * pixelWidth,
				pixelWidth,
				pixelWidth
			};

			C_RGB color = colorMap[y][x];

			SDL_SetRenderDrawColor(renderer, color.r * 255, color.g * 255, color.b * 255, 255);
			SDL_RenderFillRect(renderer, &rect);
		}
	}

	SDL_SetRenderTarget(renderer, NULL);

	return true;
}

void prerenderMapTexture(Simulation* sim, SDL_Renderer* renderer, int map_w, int map_h, int pixelWidth) {
	if (pixelWidth <= 0) pixelWidth = 1;

	uint32_t seed = std::random_device{}();
	float scale = 100.0f;
	int octaves = 5;
	float persistance = 0.5f;
	float lacunarity = 2.0f;

	std::vector<TerrainType> regions{};

	regions.push_back(TerrainType(
		"water1",
		0.3f,
		HEX_COLOR(3463C2)
	));

	regions.push_back(TerrainType(
		"water2",
		0.4f,
		HEX_COLOR(3766C6)
	));

	regions.push_back(TerrainType(
		"sand",
		0.45f,
		HEX_COLOR(BCC575)
	));

	regions.push_back(TerrainType(
		"grass1",
		0.55f,
		HEX_COLOR(579717)
	));

	regions.push_back(TerrainType(
		"grass2",
		0.6f,
		HEX_COLOR(3F6B14)
	));

	regions.push_back(TerrainType(
		"rock1",
		0.7f,
		HEX_COLOR(56413D)
	));

	regions.push_back(TerrainType(
		"rock2",
		0.9f,
		HEX_COLOR(423331)
	));

	regions.push_back(TerrainType(
		"snow",
		1.0f,
		HEX_COLOR(D1D1D1)
	));

	std::sort(regions.begin(), regions.end(), [](const TerrainType& t1, const TerrainType& t2) { return t1.height < t2.height; });

	scale /= pixelWidth;

	auto noiseMap = generateHeightMap(map_w, map_h, scale, seed, octaves, persistance, lacunarity);
	auto colorMap = generateColorMap(noiseMap, regions);

	sim->width = map_w;
	sim->height = map_h;

	bool res = renderToTexture(colorMap, sim, renderer, pixelWidth);
	if (!res) return;

	Log_Info("Successfully prerendered map texture.");
}
