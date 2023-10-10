#pragma once

#include <glm/glm.hpp>

class Camera {
private:
	float speed;

	float yaw;
	float pitch;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;

	float fov;
	float aspectRatio;
	float near;
	float far;

	glm::mat4 view;
	glm::mat4 projection;

	static constexpr float mouseSensitivity = 0.06f;
	static constexpr float scrollSensitivity = 0.6f;


	void updatePosition(float dt);

public:
	Camera(glm::vec3 position, float yaw, float pitch, float fov, int windowWidth, int windowHeight, float near, float far, float speed);
	~Camera() = default;
	
	void update(float dt);
	void mouseCallback(int xrel, int yrel);
	void scrollCallback(float y);
	void resizeCallback(int windowWidth, int windowHeight);

	glm::mat4 getView();
	glm::mat4 getProjection();
};
