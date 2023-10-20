#pragma once

#include <glad/glad.h>

#include "color.h"


class PostprocessManager {
private:
	GLuint fbo;
	
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	GLuint colorTexture;
	GLuint normalTexture;
	GLuint depthTexture;

	GLuint shaderProgram;
	GLint colorTextureLoc;
	GLint normalTextureLoc;
	GLint depthTextureLoc;
	GLint resolutionLoc;
	GLint bgColorLoc;

	static void createTexture(GLuint& texture, int w, int h, GLint internalFormat, GLenum format, GLenum type);

	void initFramebuffer(int width, int height);
	void initVAO();
	void initProgram(int width, int height, const std::string& vertex, const std::string& fragment);

public:
	PostprocessManager(int width, int height, const std::string& vertex, const std::string& fragment);
	~PostprocessManager();
	
	void Resize(int width, int height);

	void BeforeRender(const myColor::RGB& bgColor);
	void AfterRender(const myColor::RGB& bgColor);
};