#pragma once

#include <memory>

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "programManager.h"
#include "modelManager.h"
#include "camera.h"
#include "terrain.h"


class App {
private:
	bool running;
	bool freeCameraMode;

	std::unique_ptr<entt::registry> registry;
	std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
	SDL_GLContext context;
	
	std::unique_ptr<ProgramManager> prgMngr;
	std::unique_ptr<ModelManager> modelMngr;

	std::unique_ptr<Camera> camera;

	//std::unique_ptr<Terrain> terrain;

	int fps;

	void setGLAttributes();
	void createWindow(int width, int height);
	void createContext(int width, int height);

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