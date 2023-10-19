#pragma once

#include "../color.h"

namespace comps {
	struct lightEmitter {
		myColor::RGB color;

		float ambient;
		float diffuse;
		float specular;
	};

	/* The base direction is always down */
	struct dirLight {};
}