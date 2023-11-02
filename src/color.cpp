#include "color.h"

#include <stdexcept>
#include <sstream>

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

Color::XYZ RGB_to_XYZ(Color::RGB rgb) {
	float r, g, b;

	r = (rgb.r > 0.04045f) ? powf(((rgb.r + 0.055f) / 1.055f), 2.4f) : rgb.r / 12.92f;
	g = (rgb.g > 0.04045f) ? powf(((rgb.g + 0.055f) / 1.055f), 2.4f) : rgb.g / 12.92f;
	b = (rgb.b > 0.04045f) ? powf(((rgb.b + 0.055f) / 1.055f), 2.4f) : rgb.b / 12.92f;

	r *= 100.0f;
	g *= 100.0f;
	b *= 100.0f;

	// Observer = 2°, Illuminant = D65
	const float x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
	const float y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
	const float z = r * 0.0193f + g * 0.1192f + b * 0.9505f;
	
	return Color::XYZ{ x, y, z };
}

Color::LAB XYZ_to_LAB(Color::XYZ xyz) {
	float x, y, z;

	// Observer = 2°, Illuminant = D65
	x = xyz.x / 95.047f;
	y = xyz.y / 100.000f;
	z = xyz.z / 108.883f;
	
	x = (x > 0.008856f) ? powf(x, 0.333333333f) : 7.787f * x + 0.137931034f;
	y = (y > 0.008856f) ? powf(y, 0.333333333f) : 7.787f * y + 0.137931034f;
	z = (z > 0.008856f) ? powf(z, 0.333333333f) : 7.787f * z + 0.137931034f;


	const float l = (116.0f * y) - 16.0f;
	const float a = 500.0f * (x - y);
	const float b = 200.0f * (y - z);

	return Color::LAB{ l, a, b };
}

Color::LCH LAB_to_LCH(Color::LAB lab) {
	const float l = lab.l;
	const float a = lab.a;
	const float b = lab.b;

	const float c = sqrtf(a * a + b * b);

	float h = atan2f(b, a); //Quadrant by signs
	
	h = (h > 0.0f) ? (h / glm::pi<float>()) * 180.0f : 360.0f - (fabsf(h) / glm::pi<float>()) * 180.0f;

	return Color::LCH{ l, c, h };
}

Color::LAB LCH_to_LAB(Color::LCH lch) {
	const float l = lch.l;
	const float c = lch.c;
	const float h = lch.h;

	const float a = cosf(h * 0.01745329251f) * c;
	const float b = sinf(h * 0.01745329251f) * c;

	return Color::LAB{ l, a, b };
}

Color::XYZ LAB_to_XYZ(Color::LAB lab) {
	const float l = lab.l;
	const float a = lab.a;
	const float b = lab.b;

	float y = (l + 16.0f) / 116.0f;
	float x = a / 500.0f + y;
	float z = y - b / 200.0f;

	x = (x * x * x > 0.008856f) ? x = x * x * x : (x - 0.137931034f) / 7.787f;
	y = (y * y * y > 0.008856f) ? y = y * y * y : (y - 0.137931034f) / 7.787f;
	z = (z * z * z > 0.008856f) ? z = z * z * z : (z - 0.137931034f) / 7.787f;

	// Observer = 2°, Illuminant = D65
	x = 95.047f * x;
	y = 100.000f * y;
	z = 108.883f * z;

	return Color::XYZ{ x, y, z };
}

Color::RGB XYZ_to_RGB(Color::XYZ xyz) {
	// Observer = 2°, Illuminant = D65
	const float x = xyz.x / 100.0f; // X from 0 to 95.047
	const float y = xyz.y / 100.0f; // Y from 0 to 100.000
	const float z = xyz.z / 100.0f; // Z from 0 to 108.883

	float r = x * 3.2406f + y * -1.5372f + z * -0.4986f;
	float g = x * -0.9689f + y * 1.8758f + z * 0.0415f;
	float b = x * 0.0557f + y * -0.2040f + z * 1.0570f;

	r = (r > 0.0031308f) ? 1.055f * (powf(r, 0.41666667f)) - 0.055f : 12.92f * r;
	g = (g > 0.0031308f) ? 1.055f * (powf(g, 0.41666667f)) - 0.055f : 12.92f * g;
	b = (b > 0.0031308f) ? 1.055f * (powf(b, 0.41666667f)) - 0.055f : 12.92f * b;

	return Color::RGB{ r, g, b };
}

Color::RGB::RGB(float r, float g, float b) : r(r), g(g), b(b) {}
Color::RGB::RGB(int r, int g, int b) : RGB(r / 255.0f, g / 255.0f, b / 255.0f) {}
Color::RGB::RGB(float b) : RGB(b, b, b) {}

Color::RGB::RGB(const std::string& hex) {
	// Check if the string has a valid length
	if (hex.size() != 7 || hex[0] != '#') {
		throw std::invalid_argument("Invalid hex format.");
	}

	// Lambda to convert a single hex component (2 characters) to a normalized value
	auto hexToNormalizedValue = [](const std::string& s, size_t start) -> float {
		// Convert hex to decimal
		unsigned int value;
		std::stringstream ss;
		ss << std::hex << s.substr(start, 2);
		ss >> value;

		// Normalize
		return value / 255.0f;
	};

	r = hexToNormalizedValue(hex, 1);
	g = hexToNormalizedValue(hex, 3);
	b = hexToNormalizedValue(hex, 5);
}

Color::RGB::RGB() : RGB(0.0f) {}

Color::RGB::RGB(const XYZ& xyz) {
	RGB color = XYZ_to_RGB(xyz);
	r = color.r;
	g = color.g;
	b = color.b;
}
Color::RGB::RGB(const LAB& lab) {
	RGB color = XYZ_to_RGB(XYZ(lab));
	r = color.r;
	g = color.g;
	b = color.b;
}
Color::RGB::RGB(const LCH& lch) {
	RGB color = XYZ_to_RGB(XYZ(lch));
	r = color.r;
	g = color.g;
	b = color.b;
}

glm::vec3 Color::RGB::toVec3() const {
	return glm::vec3(r, g, b);
}

Color::XYZ::XYZ(float x, float y, float z) : x(x), y(y), z(z) {}
Color::XYZ::XYZ() : XYZ(0.0f, 0.0f, 0.0f) {}

Color::XYZ::XYZ(const RGB& rgb) {
	XYZ color = RGB_to_XYZ(rgb);
	x = color.x;
	y = color.y;
	z = color.z;
}
Color::XYZ::XYZ(const LAB& lab) {
	XYZ color = LAB_to_XYZ(lab);
	x = color.x;
	y = color.y;
	z = color.z;
}
Color::XYZ::XYZ(const LCH& lch) {
	XYZ color = LAB_to_XYZ(LAB(lch));
	x = color.x;
	y = color.y;
	z = color.z;
}

Color::LAB::LAB(float l, float a, float b) : l(l), a(a), b(b) {}
Color::LAB::LAB() : LAB(0.0f, 0.0f, 0.0f) {}

Color::LAB::LAB(const RGB& rgb) {
	LAB color = XYZ_to_LAB(XYZ(rgb));
	l = color.l;
	a = color.a;
	b = color.b;
}
Color::LAB::LAB(const XYZ& xyz) {
	LAB color = XYZ_to_LAB(xyz);
	l = color.l;
	a = color.a;
	b = color.b;
}
Color::LAB::LAB(const LCH& lch) {
	LAB color = LCH_to_LAB(lch);
	l = color.l;
	a = color.a;
	b = color.b;
}

Color::LCH::LCH(float l, float c, float h) : l(l), c(c), h(h) {}
Color::LCH::LCH() : LCH(0.0f, 0.0f, 0.0f) {}

Color::LCH::LCH(const RGB& rgb) {
	LCH color = LAB_to_LCH(LAB(rgb));
	l = color.l;
	c = color.c;
	h = color.h;
}
Color::LCH::LCH(const XYZ& xyz) {
	LCH color = LAB_to_LCH(LAB(xyz));
	l = color.l;
	c = color.c;
	h = color.h;

}
Color::LCH::LCH(const LAB& lab) {
	LCH color = LAB_to_LCH(lab);
	l = color.l;
	c = color.c;
	h = color.h;

}

Color::LCH Color::lerpLCH(const LCH& c1, const LCH& c2, float t) {
	Color::LCH result;
	result.l = glm::mix(c1.l, c2.l, t);
	result.c = glm::mix(c1.c, c2.c, t);
	result.h = glm::mix(c1.h, c2.h, t);
	return result;
}

Color::RGB Color::lerpRGB(const RGB& c1, const RGB& c2, float t) {
	Color::RGB result;
	result.r = glm::mix(c1.r, c2.r, t);
	result.g = glm::mix(c1.g, c2.g, t);
	result.b = glm::mix(c1.b, c2.b, t);
	return result;
}

Color::RGB Color::averageRGB(const RGB& c1, const RGB& c2) {
	Color::RGB result;
	result.r = (c1.r + c2.r);
	result.g = (c1.g + c2.g);
	result.b = (c1.b + c2.b);
	return result;
}

Color::Color()
	: rgb(), xyz(), lab(), lch()
	, rgbCalculated(true)
	, xyzCalculated(false)
	, labCalculated(false)
	, lchCalculated(false)
{}

Color::Color(const RGB& color)
	: rgb(color), xyz(), lab(), lch()
	, rgbCalculated(true)
	, xyzCalculated(false)
	, labCalculated(false)
	, lchCalculated(false)
{}

Color::Color(const XYZ& color)
	: rgb(), xyz(color), lab(), lch()
	, rgbCalculated(false)
	, xyzCalculated(true)
	, labCalculated(false)
	, lchCalculated(false)
{}

Color::Color(const LAB& color)
	: rgb(), xyz(), lab(color), lch()
	, rgbCalculated(false)
	, xyzCalculated(false)
	, labCalculated(true)
	, lchCalculated(false)
{}

Color::Color(const LCH& color)
	: rgb(), xyz(), lab(), lch(color)
	, rgbCalculated(false)
	, xyzCalculated(false)
	, labCalculated(false)
	, lchCalculated(true)
{}

Color::Color(const std::string& hex)
	: rgb(hex), xyz(), lab(), lch()
	, rgbCalculated(true)
	, xyzCalculated(false)
	, labCalculated(false)
	, lchCalculated(false)
{}

//Color::Color<Color::RGB>(float c0, float c1, float c2) {}

const Color::RGB& Color::GetRGB() {
	// calculated, doesn't need to calculate anything else
	if (rgbCalculated) {
		return rgb;
	}
	// calculation needed
	rgb = XYZ_to_RGB(GetXYZ());
	rgbCalculated = true;
	return rgb;
}

const Color::XYZ& Color::GetXYZ() {
	// calculated, doesn't need to calculate anything else
	if (xyzCalculated) {
		return xyz;
	}
	// prefer neighbor with only XYZ as its neighbor (RGB)
	else if (rgbCalculated) {
		xyz = RGB_to_XYZ(rgb);
	}
	else {
		xyz = LAB_to_XYZ(GetLAB());
	}

	xyzCalculated = true;
	return xyz;
}

const Color::LAB& Color::GetLAB() {
	// calculated, doesn't need to calculate anything else
	if (labCalculated) {
		return lab;
	}
	// prefer neighbor with only LAB as its neighbor (LCH)
	else if (lchCalculated) {
		lab = LCH_to_LAB(lch);
	}
	else {
		lab = XYZ_to_LAB(GetXYZ());
	}

	labCalculated = true;
	return lab;
}

const Color::LCH& Color::GetLCH() {
	// calculated, doesn't need to calculate anything else
	if (lchCalculated) {
		return lch;
	}
	// calculation needed
	lch = LAB_to_LCH(GetLAB());
	lchCalculated = true;
	return lch;
}


void Color::SetRGB(const RGB& color) {
	rgb = color;
	rgbCalculated = true;
	xyzCalculated = false;
	labCalculated = false;
	lchCalculated = false;
}

void Color::SetRGB(float r, float g, float b) {
	rgb = RGB{ r, g, b };
	rgbCalculated = true;
	xyzCalculated = false;
	labCalculated = false;
	lchCalculated = false;
}

void Color::SetRGB(const std::string& hex) {
	rgb = RGB{ hex };
	rgbCalculated = true;
	xyzCalculated = false;
	labCalculated = false;
	lchCalculated = false;
}

void Color::SetXYZ(const XYZ& color) {
	xyz = color;
	rgbCalculated = false;
	xyzCalculated = true;
	labCalculated = false;
	lchCalculated = false;
}

void Color::SetXYZ(float x, float y, float z) {
	xyz = XYZ(x, y, z);
	rgbCalculated = false;
	xyzCalculated = true;
	labCalculated = false;
	lchCalculated = false;
}

void Color::SetLAB(const LAB& color) {
	lab = color;
	rgbCalculated = false;
	xyzCalculated = false;
	labCalculated = true;
	lchCalculated = false;
}

void Color::SetLAB(float l, float a, float b) {
	lab = LAB(l, a, b);
	rgbCalculated = false;
	xyzCalculated = false;
	labCalculated = true;
	lchCalculated = false;
}

void Color::SetLCH(const LCH& color) {
	lch = color;
	rgbCalculated = false;
	xyzCalculated = false;
	labCalculated = false;
	lchCalculated = true;
}

void Color::SetLCH(float l, float c, float h) {
	lch = LCH(l, c, h);
	rgbCalculated = false;
	xyzCalculated = false;
	labCalculated = false;
	lchCalculated = true;
}
