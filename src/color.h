#pragma once

#include <string>
#include <stdint.h>

namespace color {
	struct XYZ;
	struct LAB;
	struct LCH;

	struct RGB {
		float r, g, b;

		RGB(float r, float g, float b);
		RGB(uint8_t r, uint8_t g, uint8_t b);
		RGB(float b);
		RGB(const std::string& hex);
		RGB();

		RGB(const XYZ& xyz);
		RGB(const LAB& lab);
		RGB(const LCH& lch);
	};

	struct XYZ {
		float x, y, z;

		XYZ(float x, float y, float z);
		XYZ();

		XYZ(const RGB& rgb);
		XYZ(const LAB& lab);
		XYZ(const LCH& lch);
	};

	struct LAB {
		float l, a, b;

		LAB(float l, float a, float b);
		LAB();

		LAB(const RGB& rgb);
		LAB(const XYZ& xyz);
		LAB(const LCH& lch);
	};

	struct LCH {
		float l, c, h;

		LCH(float l, float c, float h);
		LCH();

		LCH(const RGB& rgb);
		LCH(const XYZ& xyz);
		LCH(const LAB& lab);
	};

	RGB lerpRGB(const RGB& c1, const RGB& c2, float t);
	LCH lerpLCH(const LCH& c1, const LCH& c2, float t);
	RGB averageRGB(const RGB& c1, const RGB& c2);
}