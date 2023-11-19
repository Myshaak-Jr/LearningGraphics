#include "glModelManager.h"

#include <iostream>

#include "../comps/child.h"
#include "../comps/position.h"
#include "../comps/orientation.h"
#include "../comps/scale.h"
#include "../comps/transform.h"


GLModelManager::GLModelManager(std::shared_ptr<entt::registry> registry, std::shared_ptr<IntermediateModelManager> intermediateMngr)
	: registry(registry)
	, intermediateMngr(intermediateMngr)
{}

GLModelManager::~GLModelManager() {}


void GLModelManager::CreateInstance(entt::entity parent, const Model& model) {

	for (const auto& [meshId, materialId] : model.materialPerMesh) {
		shaderId_t shaderId = model.shaderPerMesh.at(meshId);
	
		entt::entity entity = registry->create();

		registry->emplace<comps::child>(entity, parent);

		registry->emplace<comps::position>(entity);
		registry->emplace<comps::orientation>(entity);
		registry->emplace<comps::scale>(entity);
		registry->emplace<comps::transform>(entity);

		emplaceMesh(entity, { model.objectId, meshId });
		emplaceMaterial(entity, materialId);
		emplaceShader(entity, shaderId);
	}
}

void GLModelManager::PrepareModel(const Model& model) {
	for (const auto& [meshId, shaderId] : model.shaderPerMesh) {
		ensureMeshCreated({ model.objectId, meshId });
		ensureShaderCreated(shaderId);
	}
}

const id_umap<shaderId_t, comps::shader>& GLModelManager::GetShaders() const {
	return shaders;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*      SHADER                                                                         */
/////////////////////////////////////////////////////////////////////////////////////////

void GLModelManager::emplaceShader(entt::entity entity, const shaderId_t& shaderId) {
	const comps::shader& shader = getOrCreateShader(shaderId);
	registry->emplace<comps::shader>(entity, shader);
}

GLuint GLModelManager::compileShader(const std::filesystem::path& path, GLenum shaderType) {
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
		std::stringstream ss;
		ss << "Reading failure in " << strShaderType << "shader." << std::endl;
		throw std::runtime_error(ss.str());
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

		std::stringstream ss;
		ss << "Compile failure in " << strShaderType << " shader:" << std::endl << infoLog << std::endl;
		delete[] infoLog;
		throw std::runtime_error(ss.str());
	}

	return shader;
}

void GLModelManager::linkShader(comps::shader& shader, const Shader& originalShader) {
	std::vector<GLuint> glShaders{};

	if (originalShader.vertex != "") {
		glShaders.push_back(compileShader(originalShader.vertex, GL_VERTEX_SHADER));
	}

	shader.program = glCreateProgram();

	for (GLuint glShader : glShaders) {
		glAttachShader(shader.program, glShader);
	}

	glLinkProgram(shader.program);

	// error handling
	GLint status;
	glGetProgramiv(shader.program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetProgramiv(shader.program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* infoLog = new GLchar[(size_t)infoLogLength + 1];
		glGetProgramInfoLog(shader.program, infoLogLength, NULL, infoLog);

		std::cerr << "Linker failure:" << std::endl << infoLog << std::endl;

		delete[] infoLog;
	}

	for (GLuint glShader : glShaders) {
		glDetachShader(shader.program, glShader);
	}
}

void GLModelManager::createShader(const shaderId_t& shaderId) {
	const Shader& originalShader = intermediateMngr->GetShader(shaderId);

	comps::shader shader{};

	// compile the program
	linkShader(shader, originalShader);
	
	shader.modelUnifLoc = glGetUniformLocation(shader.program, "model");
	shader.viewUnifLoc = glGetUniformLocation(shader.program, "view");
	shader.projUnifLoc = glGetUniformLocation(shader.program, "proj");
	shader.normalUnifLoc = glGetUniformLocation(shader.program, "normal");

	shader.requireLights = true;

	shaders.emplace(shaderId, shader);
}

void GLModelManager::ensureShaderCreated(const shaderId_t& shaderId) {
	if (shaders.find(shaderId) == shaders.end())
		createShader(shaderId);
}

const comps::shader& GLModelManager::getOrCreateShader(const shaderId_t& shaderId) {
	ensureShaderCreated(shaderId);
	return shaders.at(shaderId);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*      MATERIAL                                                                       */
/////////////////////////////////////////////////////////////////////////////////////////

void GLModelManager::emplaceMaterial(entt::entity entity, const materialId_t& materialId) {
	const Material& material = intermediateMngr->GetMaterial(materialId);

	switch (material.type) {
	case MaterialType::Color:
		emplaceColorMaterial(entity, material.color);
		
		break;
	case MaterialType::Texture:
		emplaceTextureMaterial(entity, material.texture);
		
		break;
	default:
		break;
	}
}

void GLModelManager::emplaceColorMaterial(entt::entity entity, const ColorData& colorData) {
	registry->emplace<comps::colorMaterial>(entity, colorData.diffuse, colorData.diffuse, colorData.specular, colorData.shininess);
}

void GLModelManager::emplaceTextureMaterial(entt::entity entity, const TextureData& textureData) {
	// TODO: ...
}

/////////////////////////////////////////////////////////////////////////////////////////
/*      MESH                                                                           */
/////////////////////////////////////////////////////////////////////////////////////////

void GLModelManager::emplaceMesh(entt::entity entity, const uniqueMeshId_t& meshId) {
	const comps::mesh& mesh = getOrCreateMesh(meshId);
	registry->emplace<comps::mesh>(entity, mesh.vao, mesh.vbo, mesh.ebo, mesh.elementCount, mesh.indexType);
}


void GLModelManager::createMesh(const uniqueMeshId_t& meshId) {
	const Mesh& originalMesh = intermediateMngr->GetObject(meshId.objectId).meshes.at(meshId.meshId);

	comps::mesh mesh = {};

	GLenum err;
	while (glGetError() != GL_NO_ERROR);

	// Generate GL objects
	glGenVertexArrays(1, &mesh.vao);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "glGenVertexArrays: OpenGL error: " << err << std::endl;
	}

	glGenBuffers(1, &mesh.vbo);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "glGenBuffers: OpenGL error: " << err << std::endl;
	}
	glGenBuffers(1, &mesh.ebo);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "glGenBuffers: OpenGL error: " << err << std::endl;
	}

	// Bind GL objects
	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);

	// Buffer data
	glBufferData(GL_ARRAY_BUFFER, originalMesh.vertices.size() * sizeof(Vertex), originalMesh.vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, originalMesh.indices.size() * sizeof(GLushort), originalMesh.indices.data(), GL_STATIC_DRAW);
	mesh.elementCount = static_cast<GLsizei>(originalMesh.indices.size());

	// Setup Pointers
	//   positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, vx)));
	//   normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, nx)));
	//   texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, tx)));

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}

	mesh.indexType = GL_UNSIGNED_SHORT;

	meshes.emplace(meshId, mesh);
}

void GLModelManager::ensureMeshCreated(const uniqueMeshId_t& meshId) {
	if (meshes.find(meshId) == meshes.end())
		createMesh(meshId);
}

const comps::mesh& GLModelManager::getOrCreateMesh(const uniqueMeshId_t& meshId) {
	ensureMeshCreated(meshId);
	return meshes.at(meshId);
}