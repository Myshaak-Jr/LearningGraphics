#include "postprocessManager.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "programManager.h"
#include "constants.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


void saveColorTextureToFile(GLuint textureID, int width, int height, const std::string& filename) {
	// Create a buffer to hold the texture data
	unsigned char* buffer = new unsigned char[width * height * 4]; // assuming 4 bytes for RGBA

	// Bind the texture and read its data into the buffer
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	// Create an SDL_Surface from the buffer
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
		buffer,
		width,
		height,
		32, // 32 bits depth
		width * 4, // pitch
		0x000000FF, // R mask
		0x0000FF00, // G mask
		0x00FF0000, // B mask
		0xFF000000  // A mask
	);

	// Save the surface to a PNG file using SDL2_image
	IMG_SavePNG(surface, filename.c_str());

	// Cleanup
	SDL_FreeSurface(surface);
	delete[] buffer;
}

void saveDepthTextureToFile(GLuint textureID, int width, int height, const std::string& filename) {
	// Create a buffer to hold the texture data
	float* buffer = new float[width * height]; // assuming 4 bytes for RGBA

	// Bind the texture and read its data into the buffer
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

	Uint8* intBuffer = new Uint8[width * height * 4];

	std::unordered_set<float> intensities;

	for (int i = 0; i < width * height; i++) {
		intensities.insert(buffer[i]);

		intBuffer[4 * i + 0] = static_cast<Uint8>(255 * buffer[i]);
		intBuffer[4 * i + 1] = static_cast<Uint8>(255 * buffer[i]);
		intBuffer[4 * i + 2] = static_cast<Uint8>(255 * buffer[i]);
		intBuffer[4 * i + 3] = 255;
	}

	// Create an SDL_Surface from the buffer
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
		intBuffer,
		width,
		height,
		32, // 32 bits depth
		width * 4, // pitch
		0x000000FF, // R mask
		0x0000FF00, // G mask
		0x00FF0000, // B mask
		0xFF000000  // A mask
	);
	if (!surface) {
		std::cerr << SDL_GetError() << std::endl;
		throw std::runtime_error("You Idiot.");
	}

	// Save the surface to a PNG file using SDL2_image
	IMG_SavePNG(surface, filename.c_str());

	// Cleanup
	SDL_FreeSurface(surface);
	delete[] buffer;
}


void PostprocessManager::createTexture(GLuint& texture, int w, int h, GLint internalFormat, GLenum format, GLenum type) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void PostprocessManager::initFramebuffer(int width, int height) {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glViewport(0, 0, width, height);
	
	// Color texture setup
	createTexture(colorTexture, width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
	
	// Normal texture setup
	createTexture(normalTexture, width, height, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);

	// Depth texture setup
	createTexture(depthTexture, width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostprocessManager::initVAO() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	const float vertices[4 * 3 + 4 * 2] = {
		// vertex       // texture
		-1.0f, +1.0f,   0.0f, 1.0f,
		-1.0f, -1.0f,   0.0f, 0.0f,
		+1.0f, -1.0f,   1.0f, 0.0f,
		+1.0f, +1.0f,   1.0f, 1.0f,

	};

	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

	const uint16_t indices[6] = {
		0, 1, 2,
		0, 2, 3,
	};

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void PostprocessManager::initProgram(int width, int height, const std::string& vertex, const std::string& fragment) {
	std::vector<GLuint> compiledShaders;
	
	compiledShaders.push_back(ProgramManager::CreateShader(GL_VERTEX_SHADER, vertex.c_str()));
	compiledShaders.push_back(ProgramManager::CreateShader(GL_FRAGMENT_SHADER, fragment.c_str()));

	shaderProgram = ProgramManager::CreateProgram(compiledShaders);

	std::for_each(compiledShaders.begin(), compiledShaders.end(), glDeleteShader);

	colorTextureLoc = glGetUniformLocation(shaderProgram, "colorTexture");
	normalTextureLoc = glGetUniformLocation(shaderProgram, "normalTexture");
	depthTextureLoc = glGetUniformLocation(shaderProgram, "depthTexture");
	resolutionLoc = glGetUniformLocation(shaderProgram, "resolution");
	bgColorLoc = glGetUniformLocation(shaderProgram, "bgColor");
	zNearLoc = glGetUniformLocation(shaderProgram, "zNear");
	zFarLoc = glGetUniformLocation(shaderProgram, "zFar");

	glUseProgram(shaderProgram);
	glUniform2f(resolutionLoc, static_cast<float>(width), static_cast<float>(height));
	glUniform1i(colorTextureLoc, 0);
	glUniform1i(normalTextureLoc, 1);
	glUniform1i(depthTextureLoc, 2);
	glUseProgram(0);
}

PostprocessManager::PostprocessManager(int width, int height, const std::string& vertex, const std::string& fragment) {
	initFramebuffer(width, height);
	initVAO();
	initProgram(width, height, vertex, fragment);
}

PostprocessManager::~PostprocessManager() {
	glDeleteProgram(shaderProgram);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorTexture);
	glDeleteTextures(1, &normalTexture);
	glDeleteTextures(1, &depthTexture);
}

void PostprocessManager::Resize(int width, int height) {
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glUseProgram(shaderProgram);
	glUniform2f(resolutionLoc, static_cast<float>(width), static_cast<float>(height));
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostprocessManager::BeforeRender(const myColor::RGB& bgColor) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (during binding framebuffer): " << err << std::endl;
	}

	glEnable(GL_DEPTH_TEST);

	glClear(GL_DEPTH_BUFFER_BIT);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glClearColor(NORMAL_MAP_DEFAULT_COLOR.r, NORMAL_MAP_DEFAULT_COLOR.g, NORMAL_MAP_DEFAULT_COLOR.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (during setting framebuffer textures): " << err << std::endl;
	}
}

void PostprocessManager::AfterRender(const myColor::RGB& bgColor, const std::unique_ptr<Camera>& camera) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render using postprocessing

	glDisable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (during binding postprocess shader): " << err << std::endl;
	}

	glUniform3f(bgColorLoc, bgColor.r, bgColor.g, bgColor.b);

	glUniform1f(zNearLoc, camera->getZNear());
	glUniform1f(zFarLoc, camera->getZFar());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (during binding postprocess textures): " << err << std::endl;
	}

	glBindVertexArray(vao);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (during binding postprocess VAO): " << err << std::endl;
	}

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error (during postprocess render): " << err << std::endl;
	}

	glBindVertexArray(0);
	glUseProgram(0);
}
