#pragma once

#include <memory>
#include <glad/glad.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "camera.h"

#include "comps/position.h"
#include "comps/rotation.h"
#include "comps/movedByKeyboard.h"
#include "comps/rotatedByKeyboard.h"


namespace systems {
	void orbitPos(const std::unique_ptr<entt::registry>& registry);

	void keyboardMove(const std::unique_ptr<entt::registry>& registry, float dt);
	template <Axis A>
	void keyboardMoveInAxis(const std::unique_ptr<entt::registry>& registry, float dt);

	void keyboardRotate(const std::unique_ptr<entt::registry>& registry, float dt);
	template <EAngle A>
	void keyboardRotateAroundAngle(const std::unique_ptr<entt::registry>& registry, float dt);

	void dynamicallyScale(const std::unique_ptr<entt::registry>& registry);

	void calcTransforms(const std::unique_ptr<entt::registry>& registry);
	void clearTransformCache(const std::unique_ptr<entt::registry>& registry);
	void calcAbsoluteTransform(const std::unique_ptr<entt::registry>& registry);

	void render(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<Camera>& camera);
}

template <Axis A>
void systems::keyboardMoveInAxis(const std::unique_ptr<entt::registry>& registry, float dt) {
	auto view = registry->view<comps::position, const comps::movedByKeyboard<A>>();

	for (auto [entity, pos, ctrl] : view.each()) {
		glm::vec3 vector = VEC_FORWARD;

		switch (A) {
		case Axis::X:
			vector = VEC_RIGHT;
			break;
		case Axis::Y:
			vector = VEC_UP;
			break;
		case Axis::Z:
			vector = VEC_FORWARD;
			break;
		default:
			break;
		}


		if (registry->all_of<comps::rotation>(entity)) {
			const comps::rotation& rot = registry->get<comps::rotation>(entity);

			switch (A) {
			case Axis::X:
				vector = glm::eulerAngleYZ(glm::radians(rot.yaw), glm::radians(rot.roll)) * glm::vec4(vector, 1.0f);
				break;
			case Axis::Y:
				vector = glm::eulerAngleXZ(glm::radians(rot.pitch), glm::radians(rot.roll)) * glm::vec4(vector, 1.0f);
				break;
			case Axis::Z:
				vector = glm::eulerAngleYX(glm::radians(rot.yaw), glm::radians(rot.pitch)) * glm::vec4(vector, 1.0f);
				break;
			default:
				break;
			}
		}

		const Uint8* state = SDL_GetKeyboardState(NULL);
		const float speed = state[SDL_SCANCODE_LCTRL] ? ctrl.speed * 3.0f : ctrl.speed;

		if (state[ctrl.inc]) {
			pos.pos += vector * speed * dt;
		}
		if (state[ctrl.dec]) {
			pos.pos -= vector * speed * dt;
		}
	}
}

template <EAngle A>
void systems::keyboardRotateAroundAngle(const std::unique_ptr<entt::registry>& registry, float dt) {
	auto view = registry->view<comps::rotation, const comps::rotatedByKeyboard<A>>();

	for (auto [entity, rot, ctrl] : view.each()) {
		const Uint8* state = SDL_GetKeyboardState(NULL);
		const float speed = state[SDL_SCANCODE_LCTRL] ? ctrl.speed * 3.0f : ctrl.speed;

		float* eulerAngle = &rot.yaw;;
		switch (A) {
		case EAngle::YAW:
			eulerAngle = &rot.yaw;
			break;
		case EAngle::PITCH:
			eulerAngle = &rot.pitch;
			break;
		case EAngle::ROLL:
			eulerAngle = &rot.roll;
			break;
		default:
			break;
		}

		if (state[ctrl.inc]) {
			*eulerAngle += speed * dt;
		}
		if (state[ctrl.dec]) {
			*eulerAngle -= speed * dt;
		}

		if (ctrl.clamped) {
			*eulerAngle = glm::clamp(*eulerAngle, ctrl.minVal, ctrl.maxVal);
		}
		else {
			*eulerAngle = glm::mod(*eulerAngle, 360.0f);
		}
	}
}