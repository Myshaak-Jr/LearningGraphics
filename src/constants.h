#pragma once

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
#define VEC_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define VEC_FORWARD glm::vec3(0.0f, 0.0f, -1.0f)

#define HEX_COLOR(h) ((((0x##h) >> 16) & 0xff) / 255.0f), ((((0x##h) >> 8) & 0xff) / 255.0f), (((0x##h) & 0xff) / 255.0f)

#define C_TREE_GREEN HEX_COLOR(0a481e)
#define C_BROWN HEX_COLOR(341a0e)

#define C_STONE1 HEX_COLOR(594f4f)
#define C_STONE2 HEX_COLOR(4a3c3c)