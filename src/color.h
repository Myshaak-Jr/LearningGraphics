#pragma once

#include <string>
#include <stdint.h>
#include <glm/glm.hpp>

namespace myColor {
	struct XYZ;
	struct LAB;
	struct LCH;

	struct RedGreenBlue {
		float r, g, b;

		RedGreenBlue(float r, float g, float b);
		/* r, g, b in range [0, 255] */
		RedGreenBlue(int r, int g, int b);
		RedGreenBlue(float b);
		RedGreenBlue(const std::string& hex);
		RedGreenBlue();

		RedGreenBlue(const XYZ& xyz);
		RedGreenBlue(const LAB& lab);
		RedGreenBlue(const LCH& lch);

		glm::vec3 toVec3() const;
	};

	struct XYZ {
		float x, y, z;

		XYZ(float x, float y, float z);
		XYZ();

		XYZ(const RedGreenBlue& rgb);
		XYZ(const LAB& lab);
		XYZ(const LCH& lch);
	};

	struct LAB {
		float l, a, b;

		LAB(float l, float a, float b);
		LAB();

		LAB(const RedGreenBlue& rgb);
		LAB(const XYZ& xyz);
		LAB(const LCH& lch);
	};

	struct LCH {
		float l, c, h;

		LCH(float l, float c, float h);
		LCH();

		LCH(const RedGreenBlue& rgb);
		LCH(const XYZ& xyz);
		LCH(const LAB& lab);
	};

	RedGreenBlue lerpRGB(const RedGreenBlue& c1, const RedGreenBlue& c2, float t);
	LCH lerpLCH(const LCH& c1, const LCH& c2, float t);
	RedGreenBlue averageRGB(const RedGreenBlue& c1, const RedGreenBlue& c2);
}