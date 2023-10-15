#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "perlin.h"
#include "modelManager.h"
#include "comps/material.h"


namespace myTerrain {
	class TerrainType {
		comps::material material;
		float height;
	};

	class Chunk {
		int x;
		int y;

		std::string modelName;
	};

	class Terrain {
	private:
		using noiseType = myMath::Perlin;

		std::string name;
		std::shared_ptr<ModelManager> modelMngr;
		std::mutex& modelMngrMtx;

		std::shared_ptr<entt::registry> registry;
		std::mutex& registryMtx;

		int chunkWidth;
		int chunkHeight;

		float scale;

		int octaves;
		float persistance;
		float lacunarity;

		uint32_t seed;

		std::vector<noiseType> perlins;
		std::vector<TerrainType> terrainTypes;

		std::vector<Chunk> loadedChunks;
		std::mutex loadedChunksMtx;

		std::vector<std::thread> chunkLoaders;

		std::condition_variable cv;
		std::queue<glm::ivec2> chunksToLoad;
		std::mutex queueMtx;

		bool stopChunkLoaders;

		void chunkLoaderJob();
		void loadChunk(glm::ivec2 chunkPos);

		std::vector<std::vector<float>> generateHeightMap(glm::ivec2 chunkPos);
		void generateMesh(std::vector<std::vector<float>> heightMap);

		void calcIndices(std::vector<uint32_t>& indices, uint32_t x, uint32_t y);



	public:
		Terrain(
			const std::string& name, std::shared_ptr<ModelManager>& modelMngr, std::mutex& modelMngrMtx,
			std::shared_ptr<entt::registry>& registry, std::mutex& registryMtx,
			const std::vector<TerrainType>& terrainTypes,
			int chunkSize, float scale, int octaves, float persistance, float lacunarity, uint32_t seed
		);
		~Terrain();

		/* Generates chunks and adds them to the registry */
		/* Multithreaded, the app should keep running while the generation happens. */
		void LoadChunks(const std::vector<glm::ivec2>& chunks);
		void LoadChunk(glm::ivec2 chunkPos);

		//void UnloadChunks();
	};
}