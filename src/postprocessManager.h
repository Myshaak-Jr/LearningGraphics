#pragma once

#include <memory>

#include <glad/glad.h>

#include "color.h"
#include "camera.h"


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
	GLint zNearLoc;
	GLint zFarLoc;

	static void createTexture(GLuint& texture, int w, int h, GLint internalFormat, GLenum format, GLenum type);

	void initFramebuffer(int width, int height);
	void initVAO();
	void initProgram(int width, int height, const std::string& vertex, const std::string& fragment);

public:
	PostprocessManager(int width, int height, const std::string& vertex, const std::string& fragment);
	~PostprocessManager();
	
	void Resize(int width, int height);

	void BeforeRender(const Color::RGB& bgColor);
	void AfterRender(const Color::RGB& bgColor, const std::unique_ptr<Camera>& camera);
};