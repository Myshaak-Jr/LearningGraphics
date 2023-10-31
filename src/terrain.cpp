#include "terrain.h"

#include <syncstream>
#include <sstream>
#include <iostream>
#include <functional>

#include "myMath.h"

#include "comps/position.h"
#include "comps/orientation.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/terrainComps.h"
#include "comps/child.h"


myTerrain::Chunk::Chunk(glm::ivec2 position, const std::string& modelName) : position(position), modelName(modelName) {}

myTerrain::Terrain::Terrain(
	const std::string& name, std::shared_ptr<ModelManager> modelMngr,
	const std::string& shaderProgramName, std::shared_ptr<const ProgramManager> prgMngr,
	std::shared_ptr<entt::registry>& registry, std::mutex& registryEntityCreateMtx,
	glm::vec3 origin, float yaw, float pitch, float roll, glm::vec3 scale,
	/*const std::vector<TerrainType>& terrainTypes,*/
	int chunkSize, float noiseScale, int octaves, float persistance, float lacunarity, uint32_t seed
)
	: name(name)
	, modelMngr(modelMngr)
	, shaderProgramName(shaderProgramName)
	, prgMngr(prgMngr)
	, registry(registry)
	, registryEntityCreateMtx(registryEntityCreateMtx)
	, chunkWidth(chunkSize)
	, chunkHeight(chunkSize)
	, noiseScale(noiseScale > 0.0f ? noiseScale : 0.0001f)
	, octaves(octaves)
	, persistance(persistance)
	, lacunarity(lacunarity)
	//, terrainTypes(terrainTypes)
	, seed(seed)
{
	initPerlins();
	initChunkLoaders();
	initRootEntity(origin, yaw, pitch, roll, scale);
}

void myTerrain::Terrain::initPerlins() {
	std::mt19937 rng{ seed };
	std::uniform_int_distribution<uint32_t> dist{};

	for (int i = 0; i < octaves; i++) {
		perlins.push_back(noiseType(dist(rng)));
	}
}

void myTerrain::Terrain::initChunkLoaders() {
	unsigned int numChunkLoaders = std::thread::hardware_concurrency();

	for (size_t i = 0; i < numChunkLoaders; ++i) {
		std::thread thread{ std::bind(&myTerrain::Terrain::chunkLoaderJob, this) };

		chunkLoaders.emplace_back(std::move(thread));
	}
}

void myTerrain::Terrain::initRootEntity(glm::vec3 origin, float yaw, float pitch, float roll, glm::vec3 scale) {
	glm::quat y = glm::angleAxis(yaw, VEC_UP);
	glm::quat x = glm::angleAxis(pitch, VEC_RIGHT);
	glm::quat z = glm::angleAxis(roll, VEC_FORWARD);
	glm::quat orient = z * x * y;

	std::unique_lock<std::mutex> lock(registryEntityCreateMtx);

	entt::entity entity = registry->create();

	registry->emplace<comps::position>(entity, origin);
	registry->emplace<comps::orientation>(entity, orient);
	registry->emplace<comps::scale>(entity, scale);
	registry->emplace<comps::transform>(entity);

	registry->emplace<comps::terrain::root>(entity);
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

	std::osyncstream(std::cout) << "Generating noise map for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;
	std::vector<std::vector<std::pair<float, glm::vec3>>> heightAndNormalMap = generateHeightAndNormalMap(chunkPos);
	std::osyncstream(std::cout) << "Generated noise map for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;

	std::string chunkName = getNameFromChunkPos(chunkPos);
	comps::material material = getMaterialFromChunkPos(chunkPos);

	std::osyncstream(std::cout) << "Generating mesh for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;
	std::unique_ptr<Mesh<uint32_t>> mesh = generateMesh("terrain", material, heightAndNormalMap);
	std::osyncstream(std::cout) << "Generated mesh for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;

	std::osyncstream(std::cout) << "Generating model for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;
	modelMngr->AddModel(chunkName);
	modelMngr->AddMeshToModel(chunkName, std::move(mesh));
	std::osyncstream(std::cout) << "Generated model for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;

	{
		std::unique_lock<std::mutex> lock(loadedChunksMtx);
		loadedChunks.push_back(Chunk{ chunkPos, chunkName });
	}
	std::osyncstream(std::cout) << "Creating entities for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;

	createEntities(chunkName, chunkPos);

	std::osyncstream(std::cout) << "Created entities for chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;

	std::osyncstream(std::cout) << "Successfully Loaded chunk [" << chunkPos.x << ", " << chunkPos.y << "]" << std::endl;
}

std::string myTerrain::Terrain::getNameFromChunkPos(glm::ivec2 chunkPos) const {
	std::stringstream ss;
	ss << name << "-chunk-[" << chunkPos.x << "," << chunkPos.y << "]";
	return ss.str();
}

comps::material myTerrain::Terrain::getMaterialFromChunkPos(glm::ivec2 chunkPos) const {
	const myColor::LCH start(myColor::RGB(50, 168, 82));
	const myColor::LCH end(myColor::RGB(194, 134, 31));

	const float t = myMath::hashPos(chunkPos, seed) / static_cast<float>(UINT32_MAX);
	const myColor::RGB color = myColor::lerpLCH(start, end, t);

	return comps::material(color, color, myColor::RGB(0.5f), 16.0f);
}

float myTerrain::Terrain::generateHeightValue(glm::vec2 samplePos) const {
	float frequency = 1;
	float maxNoiseHeight = 0.0f;
	float amplitude = 1;
	float noiseHeight = 0;

	for (int i = 0; i < octaves; i++) {
		glm::vec2 noisePos = samplePos * frequency;

		float value = perlins[i].get(noisePos);
		noiseHeight += value * amplitude;
		maxNoiseHeight += amplitude;

		amplitude *= persistance;
		frequency *= lacunarity;
	}

	return noiseHeight / maxNoiseHeight;
}

std::vector<std::vector<std::pair<float, glm::vec3>>> myTerrain::Terrain::generateHeightAndNormalMap(glm::ivec2 chunkPos) const {
	// create the height map of size + 1 to account for the fact that the height values serve as corner points
	std::vector<std::vector<std::pair<float, glm::vec3>>> heightAndNormalMap(chunkHeight + 1, std::vector<std::pair<float, glm::vec3>>(chunkWidth + 1, std::make_pair(0.0f, glm::vec3(0.0f))));

	glm::vec2 shift = glm::vec2(chunkPos) - glm::vec2(chunkWidth, chunkHeight) / 2.0f;

	for (int y = 0; y < chunkHeight + 1; y++) {
		for (int x = 0; x < chunkWidth + 1; x++) {
			glm::vec2 samplePos = (glm::vec2(x, y) + shift) / noiseScale;

			float pointHeight = generateHeightValue(samplePos);

			float delta = 0.032f;

			// point a bit to the right of the original value
			glm::vec2 posOffsetX = samplePos + glm::vec2(delta, 0.0f);
			// what is its perlin value
			float pointHeightX = generateHeightValue(posOffsetX);
			// a vector from the point to the other one, using the perlin result
			// as the third dimension
			glm::vec3 tangentX = glm::normalize(glm::vec3(samplePos.x, pointHeight, samplePos.y) - glm::vec3(posOffsetX.x, pointHeightX, posOffsetX.y));

			// same for Y
			glm::vec2 posOffsetY = samplePos + glm::vec2(0.0f, delta);
			float pointHeightY = generateHeightValue(posOffsetY);
			glm::vec3 tangentY = glm::normalize(glm::vec3(samplePos.x, pointHeight, samplePos.y) - glm::vec3(posOffsetY.x, pointHeightY, posOffsetY.y));

			glm::vec3 normal = glm::normalize(glm::cross(tangentX, tangentY));

			heightAndNormalMap[y][x] = std::make_pair(pointHeight, normal);

			printf("%.3f ", pointHeight);
		}
		printf("\n");
	}

	return heightAndNormalMap;
}

std::unique_ptr<Mesh<uint32_t>> myTerrain::Terrain::generateMesh(
	const std::string& meshName,
	const comps::material& material,
	const std::vector<std::vector<std::pair<float, glm::vec3>>>& heightAndNormalMap
) const {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	glm::vec2 shift = -glm::vec2(chunkWidth, chunkHeight) / 2.0f;

	// loop over the corners
	for (int y = 0; y < chunkHeight + 1; y++) {
		for (int x = 0; x < chunkWidth + 1; x++) {
			// generate vertex, the origin of the mesh is is the center;
			Vertex vertex = createVertex(heightAndNormalMap[y][x].first, glm::vec2(y, x) + shift, heightAndNormalMap[y][x].second);
			vertices.push_back(vertex);

			if (x == chunkWidth || y == chunkHeight) continue;
			createIndices(indices, x, y);
		}
	}

	return std::make_unique<Mesh<uint32_t>>(meshName, vertices, indices, material);
}

Vertex myTerrain::Terrain::createVertex(float height, glm::vec2 position, glm::vec3 normal) {
	Vertex vertex{};

	vertex.vx = position.x;
	vertex.vy = height;
	vertex.vz = -position.y;

	vertex.nx = normal.x;
	vertex.ny = normal.y;
	vertex.nz = -normal.z;

	return vertex;
}

void myTerrain::Terrain::createIndices(std::vector<uint32_t>& indices, int x, int y) const {
	int cornerIdxs[4] = {
		 y      * (chunkWidth + 1) +  x      ,
		 y      * (chunkWidth + 1) + (x + 1) ,
		(y + 1) * (chunkWidth + 1) + (x + 1) ,
		(y + 1) * (chunkWidth + 1) +  x      ,
	};

	indices.push_back(cornerIdxs[0]);
	indices.push_back(cornerIdxs[2]);
	indices.push_back(cornerIdxs[1]);

	indices.push_back(cornerIdxs[2]);
	indices.push_back(cornerIdxs[0]);
	indices.push_back(cornerIdxs[3]);
}

void myTerrain::Terrain::createEntities(const std::string& chunkName, glm::ivec2 chunkPos) {
	std::unique_lock<std::mutex> lock(registryEntityCreateMtx);

	entt::entity debugSphere = registry->create();

	registry->emplace<comps::position>(debugSphere, glm::vec3(chunkPos.x * chunkWidth, 5.0f, chunkPos.y * chunkHeight));
	registry->emplace<comps::orientation>(debugSphere);
	registry->emplace<comps::scale>(debugSphere);
	registry->emplace<comps::transform>(debugSphere);

	registry->emplace<comps::child>(debugSphere, rootEntity);

	auto debugSphereOffspring = modelMngr->GenEntities("sphere", debugSphere, registry);

	for (const auto& child : *debugSphereOffspring) {
		registry->emplace<comps::shaderProgram>(child.second, prgMngr->getShaderProgram(shaderProgramName));
	}

	entt::entity entity = registry->create();

	registry->emplace<comps::position>(entity, glm::vec3(chunkPos.x * chunkWidth, 0.0f, chunkPos.y * chunkHeight));
	registry->emplace<comps::orientation>(entity);
	registry->emplace<comps::scale>(entity);
	registry->emplace<comps::transform>(entity);

	registry->emplace<comps::child>(entity, rootEntity);

	auto offspring = modelMngr->GenEntities(chunkName, entity, registry);

	for (const auto& child : *offspring) {
		registry->emplace<comps::shaderProgram>(child.second, prgMngr->getShaderProgram(shaderProgramName));
	}
}