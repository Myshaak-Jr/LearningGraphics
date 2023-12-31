#pragma once

#include "color.h"

enum class Axis {
	X,
	Y,
	Z
};

enum class EAngle {
	YAW,
	PITCH,
	ROLL
};

#define VEC_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)
#define VEC_LEFT -VEC_RIGHT
#define VEC_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define VEC_DOWN -VEC_UP
#define VEC_FORWARD glm::vec3(0.0f, 0.0f, -1.0f)
#define VEC_BACKWARD -VEC_FORWARD

#define C_TREE_GREEN Color::RGB("#0a481e")
#define C_BROWN Color::RGB("#341a0e")

#define C_STONE1 Color::RGB("#594f4f")
#define C_STONE2 Color::RGB("#4a3c3c")

const Color::RGB NORMAL_MAP_DEFAULT_COLOR{ "#8080FF" };