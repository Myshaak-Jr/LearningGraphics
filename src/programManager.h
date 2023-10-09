#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "comps/shaderProgram.h"


class ProgramManager {
private:
	std::unordered_map<std::string, comps::shaderProgram> shaderPrograms;

	static GLuint createShader(GLenum shaderType, const char* path);
	static GLuint createProgram(std::vector<GLuint>& shaders);


public:
	ProgramManager() = default;
	~ProgramManager();

	void LoadShaderProgram(const std::string& name, const std::vector<std::pair<GLenum, std::string>>& shaders, const std::vector<std::string>& uniformNames);
	const comps::shaderProgram& getShaderProgram(const std::string& name) const;
};
