#include "app.h"
#include "systems.h"
#include <iostream>
#include "model.h"
#include <glm/ext/scalar_constants.hpp>
#include "constants.h"
#include "factories.h"


App::App(int width, int height) : window(nullptr, &SDL_DestroyWindow) {
	running = false;
	fps = 60;

	setGLAttributes();
	createWindow(width, height);
	createContext(width, height);

	objMngr = std::make_unique<GLObjectManager>();
	registry = std::make_unique<entt::registry>();

	loadScene();
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
}

void App::createContext(int width, int height) {
	context = SDL_GL_CreateContext(window.get());
	if (!window) {
		std::cerr << "Error creating SDL window";
	}
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	resize(width, height);

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

float App::calcFrustumScale(float fovDgr) {
	const float degToRad = 3.14159f * 2.0f / 360.0f;
	float fovRad = fovDgr * degToRad;
	return 1.0f / tan(fovRad / 2.0f);
}

void App::loadScene() {
	loadGLObjects();
	loadEntities();

	float zNear = 0.01f, zFar = 1000.0f;
	frustumScale = calcFrustumScale(45);
	cameraMatrix = glm::mat4(0.0f);

	cameraMatrix[0][0] = frustumScale;
	cameraMatrix[1][1] = frustumScale;
	cameraMatrix[2][2] = (zFar + zNear) / (zNear - zFar);
	cameraMatrix[3][2] = (2 * zFar * zNear) / (zNear - zFar);
	cameraMatrix[2][3] = -1.0f;
}

void App::loadGLObjects() {
	// load shader programs
	objMngr->addShaderProgram(0,
		{ { GL_VERTEX_SHADER, "./shaders/vertex.glsl" }, { GL_FRAGMENT_SHADER, "./shaders/fragment.glsl" } },
		{ "modelToCameraMatrix", "cameraToClipMatrix" }
	);

	// load vertex data
	objMngr->addBuffer(0, vertexData1, GL_STATIC_DRAW);
	objMngr->addBuffer(1, elementData1, GL_STATIC_DRAW);
	
	size_t colorDataOffset = sizeof(float) * 3 * numberOfVertices1;
	objMngr->addMesh(0, 0, 1,
		{
			VertAttrPtr(3, GL_FLOAT, GL_FALSE, 0, (void*)0),
			VertAttrPtr(4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset)
		}
	);
}

void App::loadEntities() {
	factories::createCrane(registry, objMngr);
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
				running = false;
			}
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				resize(event.window.data1, event.window.data2);
			}
			break;
		}
	}
}

void App::update(float dt) {
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
	systems::render(registry, cameraMatrix);

	SDL_GL_SwapWindow(window.get());
}

void App::resize(int width, int height) {
	float aspectRatio = (float)width / height;

	cameraMatrix[0][0] = frustumScale / (width >= height ? aspectRatio : 1.0f);
	cameraMatrix[1][1] = frustumScale * (width < height ? aspectRatio : 1.0f);

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}
