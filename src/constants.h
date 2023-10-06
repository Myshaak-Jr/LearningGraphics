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