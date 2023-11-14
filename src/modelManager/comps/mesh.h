#pragma once

#include <glad/glad.h>


namespace comps {
	struct mesh {
		GLuint vao;
		GLuint vbo;
		GLuint ebo;
		GLsizei elementCount;
		GLenum indexType;
	};
}