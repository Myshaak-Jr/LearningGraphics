#include "programManager.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>

// GL Object Manager

ProgramManager::~ProgramManager() {
	std::for_each(shaderPrograms.begin(), shaderPrograms.end(), [](const auto& p) { glDeleteProgram(p.second.program); });
}

void ProgramManager::LoadShaderProgram(const std::string& name, const std::vector<std::pair<GLenum, std::string>>& shaders, bool requireLights) {
	if (shaderPrograms.find(name) != shaderPrograms.end()) {
		std::stringstream ss;
		ss << "Shader program with name " << name << " already exists";
		throw std::runtime_error(ss.str());
	}

	comps::shaderProgram shaderProgram{};

	// compile the program
	std::vector<GLuint> compiledShaders;
	for (const auto& [shaderType, shaderPath] : shaders) {
		compiledShaders.push_back(CreateShader(shaderType, shaderPath.c_str()));
	}
	shaderProgram.program = CreateProgram(compiledShaders);
	std::for_each(compiledShaders.begin(), compiledShaders.end(), glDeleteShader);

	shaderProgram.modelUnifLoc = glGetUniformLocation(shaderProgram.program, "model");
	shaderProgram.viewUnifLoc = glGetUniformLocation(shaderProgram.program, "view");
	shaderProgram.projUnifLoc = glGetUniformLocation(shaderProgram.program, "proj");
	shaderProgram.normalUnifLoc = glGetUniformLocation(shaderProgram.program, "normal");

	shaderProgram.requireLights = requireLights;

	shaderPrograms.emplace(name, shaderProgram);
}

const comps::shaderProgram& ProgramManager::getShaderProgram(const std::string& name) const {
	auto it = shaderPrograms.find(name);
	if (it == shaderPrograms.end()) {
		std::stringstream ss;
		ss << "Shader program with name " << name << " not found";
		throw std::runtime_error(ss.str());
	}
	return it->second;
}

std::vector<comps::shaderProgram> ProgramManager::getShaderPrograms() {
	std::vector<comps::shaderProgram> result;
	
	for (const auto& shaderProgram : shaderPrograms) {
		result.push_back(shaderProgram.second);
	}

	return result;
}

GLuint ProgramManager::CreateShader(GLenum shaderType, const char* path) {
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

GLuint ProgramManager::CreateProgram(std::vector<GLuint>& shaders) {
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
