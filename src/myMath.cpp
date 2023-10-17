#include "myMath.h"

/* An position and seed hashing function */
uint32_t myMath::hashPos(glm::ivec2 pos, uint32_t seed) {
	uint32_t hash_x = std::hash<int>{}(pos.x);
	uint32_t hash_y = std::hash<int>{}(pos.y);
	uint32_t hash_seed = std::hash<int>{}(seed);

	// Three prime numbers
	int prime1 = 73856093;
	int prime2 = 19349663;
	int prime3 = 83492791;

	// Generate seed from x, y using prime multiplication
	uint64_t hash = (static_cast<uint64_t>(hash_x) * prime1) ^ (static_cast<uint64_t>(hash_y) * prime2) ^ (static_cast<uint64_t>(hash_seed) * prime3);

	// Mix the value using bitwise operation
	hash = (hash ^ (hash >> 30)) * 0xbf58476d1ce4e5b9;
	hash = (hash ^ (hash >> 27)) * 0x94d049bb133111eb;
	hash = hash ^ (hash >> 31);

	hash = (hash * (hash * hash * 15731 + 789221) + 1376312589);

	uint32_t value = (hash & 0xffffffff) ^ ((hash & 0xffffffff00000000) >> 32);
	
	return value;
};
