#pragma once

#include <glad/glad.h>
#include <unordered_map>
#include <string>


namespace comps {
	struct shader {
		GLuint program;

		GLint modelUnifLoc;
		GLint viewUnifLoc;
		GLint projUnifLoc;
		GLint normalUnifLoc;

		bool requireLights;
	};
}