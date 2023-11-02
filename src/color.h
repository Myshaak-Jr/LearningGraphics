#pragma once

#include <string>
#include <stdint.h>
#include <glm/glm.hpp>

class Color {
public:
	// Color spaces
	struct XYZ;
	struct LAB;
	struct LCH;

	struct RGB {
		float r, g, b;

		RGB(float r, float g, float b);
		/* r, g, b in range [0, 255] */
		RGB(int r, int g, int b);
		RGB(float b);
		RGB(const std::string& hex);
		RGB();

		RGB(const XYZ& xyz);
		RGB(const LAB& lab);
		RGB(const LCH& lch);

		glm::vec3 toVec3() const;
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

private:
	RGB rgb;
	XYZ xyz;
	LAB lab;
	LCH lch;

	bool rgbCalculated;
	bool xyzCalculated;
	bool labCalculated;
	bool lchCalculated;

public:
	Color();

	Color(const RGB& color);
	Color(const XYZ& color);
	Color(const LAB& color);
	Color(const LCH& color);

	Color(const std::string& hex);
	
	template <class TColor, std::enable_if<std::is_same<TColor, RGB>::value>>
	Color(float c0, float c1, float c2) {
		rgb = RGB(c0, c1, c2);
		rgbCalculated = true;
		xyzCalculated = false;
		labCalculated = false;
		lchCalculated = false;
	}
	template <class TColor, std::enable_if<std::is_same<TColor, XYZ>::value>>
	Color(float c0, float c1, float c2) {
		xyz = XYZ(c0, c1, c2);
		rgbCalculated = false;
		xyzCalculated = true;
		labCalculated = false;
		lchCalculated = false;
	}
	template <class TColor, std::enable_if<std::is_same<TColor, LAB>::value>>
	Color(float c0, float c1, float c2) {
		lab = LAB(c0, c1, c2);
		rgbCalculated = false;
		xyzCalculated = false;
		labCalculated = true;
		lchCalculated = false;
	}
	template <class TColor, std::enable_if<std::is_same<TColor, LCH>::value>>
	Color(float c0, float c1, float c2) {
		lch = LCH(c0, c1, c2);
		rgbCalculated = false;
		xyzCalculated = false;
		labCalculated = false;
		lchCalculated = true;
	}

	~Color() = default;

	const RGB& GetRGB();
	const XYZ& GetXYZ();
	const LAB& GetLAB();
	const LCH& GetLCH();

	void SetRGB(const RGB& color);
	void SetRGB(float r, float g, float b);
	void SetRGB(const std::string& hex);

	void SetXYZ(const XYZ& color);
	void SetXYZ(float x, float y, float z);

	void SetLAB(const LAB& color);
	void SetLAB(float l, float a, float b);

	void SetLCH(const LCH& color);
	void SetLCH(float l, float c, float h);
};