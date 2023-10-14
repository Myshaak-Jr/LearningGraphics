#include "app.h"

#include <iostream>

#include <glm/ext/scalar_constants.hpp>

#include "systems.h"
#include "constants.h"
#include "factories.h"

#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/color.h"


App::App(int width, int height) : window(nullptr, &SDL_DestroyWindow) {
	running = false;
	freeCameraMode = true;

	fps = 60;

	setGLAttributes();
	createWindow(width, height);
	createContext(width, height);

	prgMngr = std::make_unique<ProgramManager>();
	modelMngr = std::make_unique<ModelManager>();
	registry = std::make_unique<entt::registry>();
	camera = std::make_unique<Camera>(glm::vec3(0.0f, 1.8f, 0.0f), 0.0f, 0.0f, 45.0f, width, height, 0.1f, 300.0f, 10.0f);
	//terrain = std::make_unique<Terrain>(500.0f, 500.0f, 500, 3.0f);
}

App::~App() {
	SDL_DestroyWindow(window.get());
	SDL_Quit();
}

void App::setGLAttributes() {
	SDL_GL_LoadLibrary(NULL);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void App::createWindow(int width, int height) {
	window.reset(SDL_CreateWindow("A 3D Graphics Render.", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));
	if (window == nullptr) {
		std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
		exit(1);
	}

	SDL_SetRelativeMouseMode((SDL_bool)freeCameraMode);
	SDL_CaptureMouse((SDL_bool)freeCameraMode);
}

void App::createContext(int width, int height) {
	context = SDL_GL_CreateContext(window.get());
	if (!window) {
		std::cerr << "Error creating SDL window";
	}
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	// Use v-sync
	SDL_GL_SetSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	//glEnable(GL_DEPTH_CLAMP);
}

void App::loadScene() {
	loadGLObjects();
	loadEntities();
}

void App::loadGLObjects() {
	// load shader programs
	prgMngr->LoadShaderProgram("diffuse",
		{ { GL_VERTEX_SHADER, "./shaders/vertex.glsl" }, { GL_FRAGMENT_SHADER, "./shaders/fragment.glsl" } },
		true
	);

	prgMngr->LoadShaderProgram("terrain",
		{ { GL_VERTEX_SHADER, "./shaders/terrain-vertex.glsl" }, { GL_FRAGMENT_SHADER, "./shaders/terrain-fragment.glsl" } },
		false
	);

	// load models
	modelMngr->LoadModel("tree", "models/tree.obj", "models/");
	modelMngr->LoadModel("temple", "models/temple.obj", "models/");
	modelMngr->LoadModel("cube", "models/cube.obj", "models/");
}

void App::loadEntities() {
	factories::createDirLight(registry, color::RGB("#FF7777"), glm::vec3(0.0f, 15.0f, 0.0f));
	factories::createDirLight(registry, color::RGB("#77FF77"), glm::vec3(360.0f / 3.0f, 15.0f, 0.0f));
	factories::createDirLight(registry, color::RGB("#7777FF"), glm::vec3(360.0f * 2.0f / 3.0f, 15.0f, 0.0f));

	factories::createTree(registry, modelMngr, prgMngr, glm::vec3( 1.5f,  0.0f, 1.5f), glm::vec3(1.0f));
	factories::createTree(registry, modelMngr, prgMngr, glm::vec3(-1.5f, 0.0f, 1.5f), glm::vec3(1.0f));
	factories::createTree(registry, modelMngr, prgMngr, glm::vec3(1.5f, 0.0f, -1.5f), glm::vec3(1.0f));
	factories::createTree(registry, modelMngr, prgMngr, glm::vec3(-1.5f, 0.0f, -1.5f), glm::vec3(1.0f));
	
	auto temple = factories::createTemple(registry, modelMngr, prgMngr, glm::vec3(0.0f, 0.0f, -15.0f), glm::vec3(0.0f), glm::vec3(1.0f));
}

void App::run() {
	running = true;
	while (running) {
		handleEvents();

		float dt = calcDeltaTime();

		update(dt);
		render();
	}
}

float App::calcDeltaTime() {
	static Uint64 millisecsPreviusFrame = 0;

	const int millisecsPerFrame = 1000 / fps;
	Uint64 timeToWait = millisecsPerFrame - (SDL_GetTicks64() - millisecsPreviusFrame);
	if (timeToWait > 0 && timeToWait <= millisecsPerFrame)
		SDL_Delay((Uint32)timeToWait);

	float dt = (SDL_GetTicks64() - millisecsPreviusFrame) / 1000.0f;

	millisecsPreviusFrame = SDL_GetTicks64();

	return dt;

}

void App::handleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				freeCameraMode = !freeCameraMode;
				SDL_SetRelativeMouseMode((SDL_bool)freeCameraMode);
				SDL_CaptureMouse((SDL_bool)freeCameraMode);
			}
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				resize(event.window.data1, event.window.data2);
			}
			else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				running = false;
			}
			break;
		case SDL_MOUSEMOTION:
			if (freeCameraMode) camera->mouseCallback(event.motion.xrel, event.motion.yrel);
			break;
		case SDL_MOUSEWHEEL:
			if (freeCameraMode) camera->scrollCallback(event.wheel.preciseY);
			break;
		default:
			break;
		}
	}
}

void App::update(float dt) {
	if (freeCameraMode) camera->update(dt);

	systems::orbitPos(registry);

	systems::keyboardRotate(registry, dt);
	systems::keyboardMove(registry, dt);
	
	systems::dynamicallyScale(registry);
}

void App::render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	systems::calcTransforms(registry);
	systems::clearTransformCache(registry);
	systems::calcAbsoluteTransform(registry);
	systems::render(registry, camera, prgMngr);

	SDL_GL_SwapWindow(window.get());
}

void App::resize(int width, int height) {
	camera->resizeCallback(width, height);

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}
