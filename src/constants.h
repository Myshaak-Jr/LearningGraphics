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