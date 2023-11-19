#pragma once

#include <memory>
#include <glad/glad.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "camera.h"
#include "modelManager/modelManager.h"

#include "comps/position.h"
#include "comps/orientation.h"
#include "comps/movedByKeyboard.h"
#include "comps/rotatedByKeyboard.h"


namespace systems {
	void orbitPos(const std::shared_ptr<entt::registry>& registry);

	void keyboardMove(const std::shared_ptr<entt::registry>& registry, float dt);
	template <Axis A>
	void keyboardMoveInAxis(const std::shared_ptr<entt::registry>& registry, float dt);

	void keyboardRotate(const std::shared_ptr<entt::registry>& registry, float dt);
	template <EAngle A>
	void keyboardRotateAroundAngle(const std::shared_ptr<entt::registry>& registry, float dt);

	void dynamicallyScale(const std::shared_ptr<entt::registry>& registry);

	void calcTransforms(const std::shared_ptr<entt::registry>& registry);
	void clearTransformCache(const std::shared_ptr<entt::registry>& registry);
	void calcAbsoluteTransform(const std::shared_ptr<entt::registry>& registry);

	void render(const std::shared_ptr<entt::registry>& registry, const std::unique_ptr<Camera>& camera, const std::shared_ptr<ModelManager>& modelMngr);
}

template <Axis A>
void systems::keyboardMoveInAxis(const std::shared_ptr<entt::registry>& registry, float dt) {
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


		if (registry->all_of<comps::orientation>(entity)) {
			const comps::orientation& rot = registry->get<comps::orientation>(entity);

			vector = glm::mat4_cast(rot.orient) * glm::vec4(vector, 1.0f);
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
void systems::keyboardRotateAroundAngle(const std::shared_ptr<entt::registry>& registry, float dt) {
	auto view = registry->view<comps::orientation, const comps::rotatedByKeyboard<A>>();

	for (auto [entity, rot, ctrl] : view.each()) {
		const Uint8* state = SDL_GetKeyboardState(NULL);
		const float speed = state[SDL_SCANCODE_LCTRL] ? ctrl.speed * 3.0f : ctrl.speed;

		
		glm::vec3 axis{};

		switch (A) {
		case EAngle::YAW:
			axis = VEC_UP;
			break;
		case EAngle::PITCH:
			axis = VEC_RIGHT;
			break;
		case EAngle::ROLL:
			axis = VEC_FORWARD;
			break;
		default:
			break;
		}

		glm::quat offset = glm::angleAxis(0.0f, axis);

		if (state[ctrl.inc]) {
			offset = glm::angleAxis(glm::radians(speed * dt), axis);
		}
		if (state[ctrl.dec]) {
			offset = glm::angleAxis(glm::radians(-speed * dt), axis);
		}

		if (ctrl.rightMultiply) {
			rot.orient = rot.orient * offset;
		}
		else {
			rot.orient = offset * rot.orient;
		}
	}
}