#include "GLObjectManager.h"
#include <fstream>
#include <algorithm>


// GL Object Manager

GLObjectManager::~GLObjectManager() {
	std::for_each(buffers.begin(), buffers.end(), [](const auto& p) { glDeleteBuffers(1, &p.second.buffer); });
	std::for_each(meshes.begin(), meshes.end(), [](const auto& p) { glDeleteVertexArrays(1, &p.second.vao); });
	std::for_each(shaderPrograms.begin(), shaderPrograms.end(), [](const auto& p) { glDeleteProgram(p.second.program); });
}

void GLObjectManager::addMesh(Uint32 id, Uint32 vertexBufferId, Uint32 elementBufferId, const std::vector<VertAttrPtr>& attrPtrs) {
	if (meshes.find(id) != meshes.end()) {
		std::stringstream ss;
		ss << "Mesh with id " << id << " already exists";
		throw std::runtime_error(ss.str());
	}
	
	GLuint VAO;

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	Buffer vertexBuffer = getBuffer(vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.buffer);

	// set vertex attribute pointers
	for (int i = 0; i < attrPtrs.size(); i++) {
		glEnableVertexAttribArray(i);
		const auto& attrPtr = attrPtrs[i];
		glVertexAttribPointer(i, attrPtr.size, attrPtr.type, attrPtr.normalized, attrPtr.stride, attrPtr.pointer);
	}

	Buffer elementBuffer = getBuffer(elementBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer.buffer);
	
	glBindVertexArray(0);

	meshes.emplace(id, comps::mesh(VAO, elementBuffer.count));
}

const comps::mesh& GLObjectManager::getMesh(Uint32 id) const {
	auto it = meshes.find(id);
	if (it == meshes.end()) {
		std::stringstream ss;
		ss << "Vertex array with id " << id << " not found";
		throw std::runtime_error(ss.str());
	}
	return it->second;
}

void GLObjectManager::addShaderProgram(Uint32 id, const std::vector<std::pair<GLenum, std::string>>& shaders, const std::vector<std::string>& uniformNames) {
	if (shaderPrograms.find(id) != shaderPrograms.end()) {
		std::stringstream ss;
		ss << "Shader program with id " << id << " already exists";
		throw std::runtime_error(ss.str());
	}

	comps::shaderProgram shaderProgram;

	// compile the program
	std::vector<GLuint> compiledShaders;
	for (const auto& [shaderType, shaderPath] : shaders) {
		compiledShaders.push_back(createShader(shaderType, shaderPath.c_str()));
	}
	shaderProgram.program = createProgram(compiledShaders);
	std::for_each(compiledShaders.begin(), compiledShaders.end(), glDeleteShader);

	// set up the uniform locations
	for (const std::string& name : uniformNames) {
		GLint location = glGetUniformLocation(shaderProgram.program, name.c_str());
		shaderProgram.uniformLocations.emplace(name, location);
	}

	shaderPrograms.emplace(id, shaderProgram);
}

const comps::shaderProgram& GLObjectManager::getShaderProgram(Uint32 id) const {
	auto it = shaderPrograms.find(id);
	if (it == shaderPrograms.end()) {
		std::stringstream ss;
		ss << "Shader program with id " << id << " not found";
		throw std::runtime_error(ss.str());
	}
	return it->second;
}

const Buffer& GLObjectManager::getBuffer(Uint32 id) const {
	auto it = buffers.find(id);
	if (it == buffers.end()) {
		std::stringstream ss;
		ss << "Buffer with id " << id << " not found";
		throw std::runtime_error(ss.str());
	}
	return it->second;
}

GLuint GLObjectManager::createShader(GLenum shaderType, const char* path) {
	std::string strShader;
	std::ifstream shaderFile;

	// ensure ifstream objects can throw exceptions:
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		// open file
		shaderFile.open(path);
		std::stringstream shaderStream;

		// read file's buffer contents into streams
		shaderStream << shaderFile.rdbuf();

		// close file handlers
		shaderFile.close();

		// convert stream into string
		strShader = shaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::string strShaderType;
		switch (shaderType) {
		case GL_VERTEX_SHADER:
			strShaderType = "vertex";
			break;
		case GL_GEOMETRY_SHADER:
			strShaderType = "geometry";
			break;
		case GL_FRAGMENT_SHADER:
			strShaderType = "fragment";
			break;
		}
		std::cerr << "Reading failure in " << strShaderType << "shader." << std::endl;
	}

	const char* c_strShader = strShader.c_str();
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &c_strShader, NULL);

	glCompileShader(shader);

	// error handling
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* infoLog = new GLchar[(size_t)infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);

		std::string strShaderType;
		switch (shaderType) {
		case GL_VERTEX_SHADER:
			strShaderType = "vertex";
			break;
		case GL_GEOMETRY_SHADER:
			strShaderType = "geometry";
			break;
		case GL_FRAGMENT_SHADER:
			strShaderType = "fragment";
			break;
		}

		std::cerr << "Compile failure in " << strShaderType << " shader:" << std::endl << infoLog << std::endl;

		delete[] infoLog;
	}

	return shader;
}

GLuint GLObjectManager::createProgram(std::vector<GLuint>& shaders) {
	GLuint program = glCreateProgram();

	for (auto shader : shaders) {
		glAttachShader(program, shader);
	}

	glLinkProgram(program);

	// error handling
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* infoLog = new GLchar[(size_t)infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);

		std::cerr << "Linker failure:" << std::endl << infoLog << std::endl;

		delete[] infoLog;
	}

	for (auto shader : shaders) {
		glDetachShader(program, shader);
	}

	return program;
}

// Vertex Attribute Pointer

VertAttrPtr::VertAttrPtr(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) :
	size(size),
	type(type),
	normalized(normalized),
	stride(stride),
	pointer(pointer)
{}