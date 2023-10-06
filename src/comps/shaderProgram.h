#pragma once

#include <glad/glad.h>
#include <unordered_map>
#include <string>


namespace comps {
	struct shaderProgram {
		GLuint program;
		std::unordered_map<std::string, GLint> uniformLocations;
	};
}