#include "hashHelper.h"

size_t combine_hash(size_t lhs, size_t rhs) {
	if constexpr (sizeof(size_t) >= 8) {
		lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
	}
	else {
		lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
	}
	return lhs;
}


bool CompareIndex::operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const noexcept {
	return (
		a.vertex_index == b.vertex_index &&
		a.normal_index == b.normal_index &&
		a.texcoord_index == b.texcoord_index
	);
}

size_t HashIndex::operator()(const tinyobj::index_t& index) const noexcept {
	size_t vertexHash = std::hash<int>{}(index.vertex_index);
	size_t normalHash = std::hash<int>{}(index.normal_index);
	size_t texcoordHash = std::hash<int>{}(index.texcoord_index);
	return combine_hash(combine_hash(vertexHash, normalHash), texcoordHash);
}

