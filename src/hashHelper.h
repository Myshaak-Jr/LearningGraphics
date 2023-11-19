#pragma once

#include <tinyobj/tiny_obj_loader.h>


size_t combine_hash(size_t lhs, size_t rhs);

struct CompareIndex {
	bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const noexcept;
};

struct HashIndex {
	size_t operator()(const tinyobj::index_t& index) const noexcept;
};
