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
#include "programManager.h"
#include "comps/material.h"


namespace myTerrain {
	class TerrainType {
		comps::material material;
		float height;
	};

	class Chunk {
		glm::ivec2 position;
		std::string modelName;

	public:
		Chunk(glm::ivec2 position, const std::string& modelName);
	};

	class Terrain {
	private:
		using noiseType = myMath::Perlin;

		std::string name;
		std::shared_ptr<ModelManager> modelMngr;

		std::shared_ptr<const ProgramManager> prgMngr;
		std::string shaderProgramName;

		std::shared_ptr<entt::registry> registry;
		std::mutex& registryEntityCreateMtx;

		entt::entity rootEntity;

		int chunkWidth;
		int chunkHeight;

		float noiseScale;

		int octaves;
		float persistance;
		float lacunarity;

		uint32_t seed;

		std::vector<noiseType> perlins;
		//std::vector<TerrainType> terrainTypes;

		std::vector<Chunk> loadedChunks;
		std::mutex loadedChunksMtx;

		std::vector<std::thread> chunkLoaders;

		std::condition_variable cv;
		std::queue<glm::ivec2> chunksToLoad;
		std::mutex queueMtx;

		bool stopChunkLoaders;

		void initPerlins();
		void initChunkLoaders();
		void initRootEntity(glm::vec3 origin, float yaw, float pitch, float roll, glm::vec3 scale);

		void chunkLoaderJob();

		std::string getNameFromChunkPos(glm::ivec2 chunkPos) const;
		comps::material getMaterialFromChunkPos(glm::ivec2 chunkPos) const;

		float generateHeightValue(glm::vec2 samplePos) const;
		std::vector<std::vector<std::pair<float, glm::vec3>>> generateHeightAndNormalMap(glm::ivec2 chunkPos) const;
		std::unique_ptr<Mesh<uint32_t>> generateMesh(const std::string& meshName, const comps::material& material, const std::vector<std::vector<std::pair<float, glm::vec3>>>& heightAndNormalMap) const;

		static Vertex createVertex(float height, glm::vec2 position, glm::vec3 normal);
		void createIndices(std::vector<uint32_t>& indices, int x, int y) const;

		void createEntities(const std::string& name, glm::ivec2 chunkPos);

	public:
		Terrain(
			const std::string& name, std::shared_ptr<ModelManager> modelMngr,
			const std::string& shaderProgramName, std::shared_ptr<const ProgramManager> prgMngr,
			std::shared_ptr<entt::registry>& registry, std::mutex& registryEntityCreateMtx,
			glm::vec3 origin, float yaw, float pitch, float roll, glm::vec3 scale,
			/*const std::vector<TerrainType>& terrainTypes,*/
			int chunkSize, float noiseScale, int octaves, float persistance, float lacunarity, uint32_t seed
		);
		~Terrain();
		void loadChunk(glm::ivec2 chunkPos);

		/* Generates chunks and adds them to the registry */
		/* Multithreaded, the app should keep running while the generation happens. */
		void LoadChunks(const std::vector<glm::ivec2>& chunks);
		void LoadChunk(glm::ivec2 chunkPos);

		//void UnloadChunks();
	};
}