#pragma once

#include "../color.h"

namespace comps {
	struct material {
		myColor::RedGreenBlue ambient;
		myColor::RedGreenBlue diffuse;
		myColor::RedGreenBlue specular;
		float shininess;

		inline material(myColor::RedGreenBlue ambient, myColor::RedGreenBlue diffuse, myColor::RedGreenBlue specular, float shininess)
			: ambient(ambient)
			, diffuse(diffuse)
			, specular(specular)
			, shininess(shininess)
		{}

		inline material() : material(myColor::RedGreenBlue(0.0f), myColor::RedGreenBlue(0.0f), myColor::RedGreenBlue(0.0f), 1.0f) {}
	};
}