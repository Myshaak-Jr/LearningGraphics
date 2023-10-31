#pragma once

#include <memory>
#include <mutex>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "programManager.h"
#include "modelManager.h"
#include "postprocessManager.h"
#include "camera.h"


class App {
private:
	bool running;
	bool freeCameraMode;

	std::shared_ptr<entt::registry> registry;
	std::mutex registryEntityCreateMtx;
	
	std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
	SDL_GLContext context;
	
	std::shared_ptr<ProgramManager> prgMngr;
	std::shared_ptr<ModelManager> modelMngr;

	std::unique_ptr<Camera> camera;

	std::unique_ptr<PostprocessManager> postprocess;

	float normalThreshold = 0.01f;

	//std::unique_ptr<Terrain> terrain;

	int fps;

	void setGLAttributes();
	void createWindow(int width, int height);
	void createContext(int width, int height);
	void createFramebuffer(int width, int height);

	void loadGLObjects();
	
	void loadEntities();

	float calcDeltaTime();

	void handleEvents();
	void update(float dt);
	void render();

	void resize(int width, int height);
public:
	App(int width, int height);
	~App();

	void loadScene();
	void run();
};