#pragma once

#include "../../color.h"

namespace comps {
	struct colorMaterial {
		Color ambient;
		Color diffuse;
		Color specular;
		float shininess;

		inline colorMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess)
			: ambient(ambient)
			, diffuse(diffuse)
			, specular(specular)
			, shininess(shininess)
		{}

		inline colorMaterial() : colorMaterial(Color::RGB(0.0f), Color::RGB(0.0f), Color::RGB(0.0f), 1.0f) {}
	};
}