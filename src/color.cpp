#include "color.h"

#include <stdexcept>
#include <sstream>

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

color::XYZ RGB_to_XYZ(color::RGB rgb) {
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
	
	return color::XYZ{ x, y, z };
}

color::LAB XYZ_to_LAB(color::XYZ xyz) {
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

	return color::LAB{ l, a, b };
}

color::LCH LAB_to_LCH(color::LAB lab) {
	const float l = lab.l;
	const float a = lab.a;
	const float b = lab.b;

	const float c = sqrtf(a * a + b * b);

	float h = atan2f(b, a); //Quadrant by signs
	
	h = (h > 0.0f) ? (h / glm::pi<float>()) * 180.0f : 360.0f - (fabsf(h) / glm::pi<float>()) * 180.0f;

	return color::LCH{ l, c, h };
}

color::LAB LCH_to_LAB(color::LCH lch) {
	const float l = lch.l;
	const float c = lch.c;
	const float h = lch.h;

	const float a = cosf(h * 0.01745329251f) * c;
	const float b = sinf(h * 0.01745329251f) * c;

	return color::LAB{ l, a, b };
}

color::XYZ LAB_to_XYZ(color::LAB lab) {
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

	return color::XYZ{ x, y, z };
}

color::RGB XYZ_to_RGB(color::XYZ xyz) {
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

	return color::RGB{ r, g, b };
}

color::RGB::RGB(float r, float g, float b) : r(r), g(g), b(b) {}
color::RGB::RGB(uint8_t r, uint8_t g, uint8_t b) : RGB(r / 255.0f, g / 255.0f, b / 255.0f) {}
color::RGB::RGB(float b) : RGB(b, b, b) {}

color::RGB::RGB(const std::string& hex) {
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

color::RGB::RGB() : RGB(0.0f) {}

color::RGB::RGB(const XYZ& xyz) {
	RGB color = XYZ_to_RGB(xyz);
	r = color.r;
	g = color.g;
	b = color.b;
}
color::RGB::RGB(const LAB& lab) {
	RGB color = XYZ_to_RGB(XYZ(lab));
	r = color.r;
	g = color.g;
	b = color.b;
}
color::RGB::RGB(const LCH& lch) {
	RGB color = XYZ_to_RGB(XYZ(lch));
	r = color.r;
	g = color.g;
	b = color.b;
}

color::XYZ::XYZ(float x, float y, float z) : x(x), y(y), z(z) {}
color::XYZ::XYZ() : XYZ(0.0f, 0.0f, 0.0f) {}

color::XYZ::XYZ(const RGB& rgb) {
	XYZ color = RGB_to_XYZ(rgb);
	x = color.x;
	y = color.y;
	z = color.z;
}
color::XYZ::XYZ(const LAB& lab) {
	XYZ color = LAB_to_XYZ(lab);
	x = color.x;
	y = color.y;
	z = color.z;
}
color::XYZ::XYZ(const LCH& lch) {
	XYZ color = LAB_to_XYZ(LAB(lch));
	x = color.x;
	y = color.y;
	z = color.z;
}

color::LAB::LAB(float l, float a, float b) : l(l), a(a), b(b) {}
color::LAB::LAB() : LAB(0.0f, 0.0f, 0.0f) {}

color::LAB::LAB(const RGB& rgb) {
	LAB color = XYZ_to_LAB(XYZ(rgb));
	l = color.l;
	a = color.a;
	b = color.b;
}
color::LAB::LAB(const XYZ& xyz) {
	LAB color = XYZ_to_LAB(xyz);
	l = color.l;
	a = color.a;
	b = color.b;
}
color::LAB::LAB(const LCH& lch) {
	LAB color = LCH_to_LAB(lch);
	l = color.l;
	a = color.a;
	b = color.b;
}

color::LCH::LCH(float l, float c, float h) : l(l), c(c), h(h) {}
color::LCH::LCH() : LCH(0.0f, 0.0f, 0.0f) {}

color::LCH::LCH(const RGB& rgb) {
	LCH color = LAB_to_LCH(LAB(rgb));
	l = color.l;
	c = color.c;
	h = color.h;
}
color::LCH::LCH(const XYZ& xyz) {
	LCH color = LAB_to_LCH(LAB(xyz));
	l = color.l;
	c = color.c;
	h = color.h;

}
color::LCH::LCH(const LAB& lab) {
	LCH color = LAB_to_LCH(lab);
	l = color.l;
	c = color.c;
	h = color.h;

}

color::LCH color::lerpLCH(const LCH& c1, const LCH& c2, float t) {
	color::LCH result;
	result.l = glm::mix(c1.l, c2.l, t);
	result.c = glm::mix(c1.c, c2.c, t);
	result.h = glm::mix(c1.h, c2.h, t);
	return result;
}

color::RGB color::lerpRGB(const RGB& c1, const RGB& c2, float t) {
	color::RGB result;
	result.r = glm::mix(c1.r, c2.r, t);
	result.g = glm::mix(c1.g, c2.g, t);
	result.b = glm::mix(c1.b, c2.b, t);
	return result;
}

color::RGB color::averageRGB(const RGB& c1, const RGB& c2) {
	color::RGB result;
	result.r = (c1.r + c2.r);
	result.g = (c1.g + c2.g);
	result.b = (c1.b + c2.b);
	return result;
}