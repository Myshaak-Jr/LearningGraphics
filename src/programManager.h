#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "comps/shaderProgram.h"


class ProgramManager {
private:
	std::unordered_map<std::string, comps::shaderProgram> shaderPrograms;

public:
	ProgramManager() = default;
	~ProgramManager();

	void LoadShaderProgram(const std::string& name, const std::vector<std::pair<GLenum, std::string>>& shaders, bool requireLights);
	const comps::shaderProgram& getShaderProgram(const std::string& name) const;
	std::vector<comps::shaderProgram> getShaderPrograms();

	static GLuint CreateShader(GLenum shaderType, const char* path);
	static GLuint CreateProgram(std::vector<GLuint>& shaders);
};
