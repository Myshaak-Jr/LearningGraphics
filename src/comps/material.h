#pragma once

#include "../color.h"

namespace comps {
	struct material {
		myColor::RGB ambient;
		myColor::RGB diffuse;
		myColor::RGB specular;
		float shininess;

		inline material(myColor::RGB ambient, myColor::RGB diffuse, myColor::RGB specular, float shininess)
			: ambient(ambient)
			, diffuse(diffuse)
			, specular(specular)
			, shininess(shininess)
		{}

		inline material() : material(myColor::RGB(0.0f), myColor::RGB(0.0f), myColor::RGB(0.0f), 1.0f) {}
	};
}