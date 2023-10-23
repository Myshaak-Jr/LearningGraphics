#include "camera.h"

#include <iostream>

#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#include "constants.h"


Camera::Camera(glm::vec3 position, float yaw, float pitch, float fov, int windowWidth, int windowHeight, float near, float far, float speed)
	: position(position)
	, speed(speed)
	, fov(fov)
	, yaw(yaw)
	, pitch(pitch)
	, aspectRatio((float)windowWidth / (float)windowHeight)
	, near(near)
	, far(far)
	, front(VEC_FORWARD)
	, up(VEC_UP)
	, view(glm::mat4(1.0f))
	, projection(glm::perspective(glm::radians(fov), aspectRatio, near, far))
{}

void Camera::updatePosition(float dt) {
	const Uint8* state = SDL_GetKeyboardState(nullptr);
	
	if (state[SDL_SCANCODE_W]) {
		position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * speed * dt;
	}
	if (state[SDL_SCANCODE_S]) {
		position -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * speed * dt;
	}
	if (state[SDL_SCANCODE_A]) {
		position -= glm::normalize(glm::cross(glm::vec3(front.x, 0.0f, front.z), up)) * speed * dt;
	}
	if (state[SDL_SCANCODE_D]) {
		position += glm::normalize(glm::cross(glm::vec3(front.x, 0.0f, front.z), up)) * speed * dt;
	}
	if (state[SDL_SCANCODE_SPACE]) {
		position += up * speed * dt;
	}
	if (state[SDL_SCANCODE_LSHIFT]) {
		position -= up * speed * dt;
	}
}

void Camera::update(float dt) {
	updatePosition(dt);

	view = glm::lookAt(position, position + front, up);
}

void Camera::mouseCallback(int xrel, int yrel) {
	yaw += xrel * mouseSensitivity;
	pitch -= yrel * mouseSensitivity;
	
	pitch = glm::clamp(pitch, -89.f, 89.f);

	glm::vec3 direction = glm::vec3(
		sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		-cos(glm::radians(yaw)) * cos(glm::radians(pitch))
	);

	//glm::vec3 direction(
	//	cos(glm::radians(yaw)) ),
	//	sin(glm::radians(pitch)),
	//	sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	//);
	front = glm::normalize(direction);
}

void Camera::scrollCallback(float y) {
	fov -= y * scrollSensitivity;

	fov = glm::clamp(fov, 1.f, 45.f);

	projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

void Camera::resizeCallback(int windowWidth, int windowHeight) {
	aspectRatio = (float)windowWidth / (float)windowHeight;
	projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
}

glm::mat4 Camera::getView() const {
	return view;
}

glm::mat4 Camera::getProjection() const {
	return projection;
}

float Camera::getZNear() const {
	return near;
}

float Camera::getZFar() const {
	return far;
}
