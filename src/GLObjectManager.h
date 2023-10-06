#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <SDL2/SDL_stdinc.h>
#include <iostream>
#include <sstream>
#include "comps/mesh.h"
#include "comps/shaderProgram.h"


struct VertAttrPtr {
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	const void* pointer;

	VertAttrPtr(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
};

struct Buffer {
	GLuint buffer;
	GLsizei count;
};

class GLObjectManager {
private:
	std::unordered_map<Uint32, Buffer> buffers;
	std::unordered_map<Uint32, comps::mesh> meshes;
	std::unordered_map<Uint32, comps::shaderProgram> shaderPrograms;

	static GLuint createShader(GLenum shaderType, const char* path);
	static GLuint createProgram(std::vector<GLuint>& shaders);


public:
	GLObjectManager() = default;
	~GLObjectManager();

	template <class T>
	void addBuffer(Uint32 id, const std::vector<T>& data, GLenum usage);
	void addMesh(Uint32 id, Uint32 vertexBufferId, Uint32 elementBufferId, const std::vector<VertAttrPtr>& attrPtrs);
	void addShaderProgram(Uint32 id, const std::vector<std::pair<GLenum, std::string>>& shaders, const std::vector<std::string>& uniformNames);

	const Buffer& getBuffer(Uint32 id) const;
	const comps::mesh& getMesh(Uint32 id) const;
	const comps::shaderProgram& getShaderProgram(Uint32 id) const;
};

template <class T>
void GLObjectManager::addBuffer(Uint32 id, const std::vector<T>& data, GLenum usage) {
	if (buffers.find(id) != buffers.end()) {
		std::stringstream ss;
		ss << "Buffer with id " << id << " already exists";
		throw std::runtime_error(ss.str());
	}

	GLuint buffer;
	glGenBuffers(1, &buffer);

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	const auto& result = buffers.emplace(id, Buffer(buffer, (GLsizei)data.size()));
}