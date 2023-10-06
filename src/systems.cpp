#include "systems.h"
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <iostream>
#include <stack>
#include <unordered_set>

#include "comps/child.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/dynamicallyScaled.h"
#include "comps/orbiting.h"

void systems::orbitPos(const std::unique_ptr<entt::registry>& registry) {
	auto view = registry->view<const comps::orbiting, comps::position>();
	
	for (auto [entity, orbit, pos] : view.each()) {
		const glm::vec3 scale = glm::pi<float>() / orbit.duration;
		pos.pos = orbit.amplitude * glm::sin((SDL_GetTicks64() / 1000.0f) * scale + orbit.phase) + orbit.center;
	}
}

void systems::keyboardMove(const std::unique_ptr<entt::registry>& registry, float dt) {
	keyboardMoveInAxis<Axis::X>(registry, dt);
	keyboardMoveInAxis<Axis::Y>(registry, dt);
	keyboardMoveInAxis<Axis::Z>(registry, dt);
}

void systems::keyboardRotate(const std::unique_ptr<entt::registry>& registry, float dt) {
	keyboardRotateAroundAngle<EAngle::YAW>(registry, dt);
	keyboardRotateAroundAngle<EAngle::PITCH>(registry, dt);
	keyboardRotateAroundAngle<EAngle::ROLL>(registry, dt);
}

void systems::dynamicallyScale(const std::unique_ptr<entt::registry>& registry) {
	auto view = registry->view<comps::scale, const comps::dynamicallyScaled>();
	for (auto [e, scale, dynScale] : view.each()) {
		glm::vec3 lerpFactor = glm::mod(glm::vec3(SDL_GetTicks64() / 1000.0f), dynScale.duration) / dynScale.duration;

		if (lerpFactor.x > 0.5f) lerpFactor.x = 1.0f - lerpFactor.x;
		if (lerpFactor.y > 0.5f) lerpFactor.y = 1.0f - lerpFactor.y;
		if (lerpFactor.z > 0.5f) lerpFactor.z = 1.0f - lerpFactor.z;

		lerpFactor *= 2.0f;

		scale.scl = glm::mix(dynScale.start, dynScale.end, lerpFactor);
	}
}

/* Render */

void systems::calcTransforms(const std::unique_ptr<entt::registry>& registry) {
	auto view = registry->view<comps::transform, const comps::position, const comps::rotation, const comps::scale>();

	for (auto [e, transform, pos, rot, scl] : view.each()) {
		transform.matrix =
			  glm::translate(glm::mat4(1.0f), pos.pos + rot.center)
			* glm::eulerAngleYXZ(glm::radians(rot.yaw), glm::radians(rot.pitch), glm::radians(rot.roll))
			* glm::translate(glm::mat4(1.0f), -rot.center)
			* glm::scale(glm::mat4(1.0f), scl.scl);
	}
}

void systems::clearTransformCache(const std::unique_ptr<entt::registry>& registry) {
	auto view = registry->view<comps::child>();

	for (auto [e, child] : view.each()) {
		child.hasBeenCalculated = false;
	}
}

void systems::calcAbsoluteTransform(const std::unique_ptr<entt::registry>& registry) {
	auto view = registry->view<comps::child, comps::transform>();

	for (auto [entity, child, transform] : view.each()) {
		if (child.hasBeenCalculated) continue;
		
		/* a matrix stack for computing absolute transform of this child */
		std::stack<comps::transform*> matrixStack;
		/* an absolute parent */
		glm::mat4 baseMatrix;

		/* a set for handling circular parent-child hierarchy */
		std::unordered_set<entt::entity> visitedEntities;
		
		entt::entity currEntity = entity;

		/* load parent entities */
		while (true) {
			if (visitedEntities.find(currEntity) != visitedEntities.end()) {
				/* circular hierarchy */
				throw std::runtime_error("Entity parent-child hierarchy is circular.");
			}
			visitedEntities.insert(currEntity);

			if (!registry->all_of<comps::transform>(currEntity)) {
				/* perform a step "back" to last child, because this parent
				   shan't be used as base */
				/* cannot fail, the matrix stack will always have at least
				   the starting child, that will always have transform
				   component */
				baseMatrix = matrixStack.top()->matrix;
				matrixStack.pop();
			}

			comps::transform* currTransform = &registry->get<comps::transform>(currEntity);
			comps::child* currChild;

			if (!registry->all_of<comps::child>(currEntity) || (currChild = &registry->get<comps::child>(currEntity))->hasBeenCalculated) {
				/* absolute or precalculated parent found, se it as the base */
				baseMatrix = currTransform->matrix;
				break;
			}

			/* to be calculated */
			matrixStack.push(currTransform);
			/* mark as calculated */
			currChild->hasBeenCalculated = true;
			/* propagate to the next parent */
			currEntity = currChild->parent;
		}

		/* calculate the stack */
		while (!matrixStack.empty()) {
			comps::transform* t = matrixStack.top();
			t->matrix = baseMatrix = baseMatrix * t->matrix;
			matrixStack.pop();
		}
	}
}

void systems::render(const std::unique_ptr<entt::registry>& registry, const glm::mat4& cameraMatrix) {
	auto view = registry->view<const comps::mesh, const comps::shaderProgram, const comps::transform>();

	for (auto [entity, mesh, prg, transform] : view.each()) {
		GLenum err;
		while (glGetError() != GL_NO_ERROR);

		glUseProgram(prg.program);
		glBindVertexArray(mesh.vao);

		glUniformMatrix4fv(prg.uniformLocations.at("modelToCameraMatrix"), 1, GL_FALSE, glm::value_ptr(transform.matrix));
		glUniformMatrix4fv(prg.uniformLocations.at("cameraToClipMatrix"), 1, GL_FALSE, glm::value_ptr(cameraMatrix));

		glDrawElements(GL_TRIANGLES, mesh.elementCount, GL_UNSIGNED_SHORT, 0);

		glBindVertexArray(0);
		glUseProgram(0);

		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error: " << err << std::endl;
		}
	}
}