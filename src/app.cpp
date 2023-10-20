#include "app.h"

#include <iostream>
#include <random>

#include <glm/ext/scalar_constants.hpp>

#include "systems.h"
#include "constants.h"
#include "factories.h"

#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/material.h"


App::App(int width, int height) : window(nullptr, &SDL_DestroyWindow) {
	running = false;
	freeCameraMode = true;

	fps = 60;

	setGLAttributes();
	createWindow(width, height);
	createContext(width, height);

	prgMngr = std::make_shared<ProgramManager>();
	modelMngr = std::make_shared<ModelManager>();
	registry = std::make_shared<entt::registry>();
	camera = std::make_unique<Camera>(glm::vec3(0.0f, 1.8f, 20.0f), 0.0f, 0.0f, 45.0f, width, height, 1.0f, 300.0f, 10.0f);
	postprocess = std::make_unique<PostprocessManager>(width, height, "./shaders/postprocess-vertex.glsl", "./shaders/postprocess-fragment.glsl");
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
	glFrontFace(GL_CCW);
	//glEnable(GL_DEPTH_CLAMP);
}

void App::loadScene() {
	loadGLObjects();
	loadEntities();
}

void App::loadGLObjects() {
	// load shader programs
	prgMngr->LoadShaderProgram("phong-lighting",
		{ { GL_VERTEX_SHADER, "./shaders/model.glsl" }, { GL_FRAGMENT_SHADER, "./shaders/phong-lighting.glsl" } },
		true
	);

	// load models
	//modelMngr->LoadModel("tree", "models/tree.obj", "models/");
	//modelMngr->LoadModel("temple", "models/temple.obj", "models/");
	modelMngr->LoadModel("sphere", "models/sphere.obj", "models/");
	modelMngr->LoadModel("temple", "models/temple.obj", "models/");
}

void App::loadEntities() {
	std::unique_lock<std::mutex> lock(registryEntityCreateMtx);

	//const int NUM_LIGHTS = 3;
	//myColor::RGB lightColors[NUM_LIGHTS] = { myColor::RGB("#FFFF00"), myColor::RGB("#00FFFF"), myColor::RGB("#FF00FF") };

	//for (int i = 0; i < NUM_LIGHTS; i++) {
	//	auto light = factories::createDirLight(registry, lightColors[i],
	//		0.01f, 0.5f, 0.7f,
	//		360.0f * static_cast<float>(i) / static_cast<float>(NUM_LIGHTS) + 30.0f, 30.0f, 0.0f
	//	);
	//	registry->emplace<comps::rotatedByKeyboard<EAngle::YAW>>(light, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, 90.0f, false);
	//}

	auto light = factories::createDirLight(registry, myColor::RGB("#FFFFFF"),
		0.1f, 0.5f, 0.7f,
		30.0f, 30.0f, 0.0f
	);
	registry->emplace<comps::rotatedByKeyboard<EAngle::YAW>>(light, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, 90.0f, false);


	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> dist{-5.0f, 5.0f};

	const int numSpheres = 5;
	
	const myColor::LCH start = myColor::RGB("#2dbd54");
	const myColor::LCH end = myColor::RGB("#bd942d");

	for (int i = 0; i < numSpheres; i++) {
		myColor::RGB color = myColor::lerpLCH(start, end, static_cast<float>(i) / static_cast<float>(numSpheres));
		factories::createSphere(registry, modelMngr, prgMngr, glm::vec3(dist(rng), dist(rng), dist(rng)), glm::vec3(0.0f), glm::vec3(1.0f), color);
	}

	factories::createTemple(registry, modelMngr, prgMngr, glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.0f), glm::vec3(1.0f));
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

	systems::calcTransforms(registry);
	systems::clearTransformCache(registry);
	systems::calcAbsoluteTransform(registry);
}

void App::render() {
	myColor::RGB bgColor = myColor::RGB("#615d54");

	//glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
	//glClearDepth(1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	postprocess->BeforeRender(bgColor);
	systems::render(registry, camera, prgMngr);
	postprocess->AfterRender(bgColor);

	SDL_GL_SwapWindow(window.get());
}

void App::resize(int width, int height) {
	camera->resizeCallback(width, height);
	postprocess->Resize(width, height);

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}
