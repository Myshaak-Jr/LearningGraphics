#include "systems.h"
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <iostream>
#include <stack>
#include <unordered_set>
#include <sstream>

#include "comps/child.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/dynamicallyScaled.h"
#include "comps/orbiting.h"
#include "comps/mesh.h"
#include "comps/shaderProgram.h"
#include "comps/color.h"
#include "comps/dirLight.h"


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
	auto view = registry->view<comps::transform, const comps::position, const comps::orientation, const comps::scale>();

	for (auto [e, transform, pos, rot, scl] : view.each()) {
		transform.matrix =
			  glm::translate(glm::mat4(1.0f), pos.pos + rot.center)
			* glm::mat4_cast(rot.orient)
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

void setDirLightUniforms(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<ProgramManager>& prgMngr) {
	auto view = registry->view<const comps::dirLight>();

	for (const auto& prg : prgMngr->getShaderPrograms()) {
		if (!prg.requireLights) continue;
		glUseProgram(prg.program);

		int i = 0;

		for (auto [entity, light] : view.each()) {
			std::stringstream ss;
			ss << "dirLights[" << i++ << "]";
			std::string locName = ss.str();

			glm::vec3 dir = glm::normalize(light.baseDir);

			if (registry->all_of<comps::orientation>(entity)) {
				const comps::orientation& orient = registry->get<comps::orientation>(entity);
				dir = orient.orient * dir;
			}


			GLint directionLoc = glGetUniformLocation(prg.program, (locName + ".dir").c_str());
			GLint diffuseLoc = glGetUniformLocation(prg.program, (locName + ".diffuse").c_str());

			if (directionLoc != -1) {
				glUniform3fv(directionLoc, 1, glm::value_ptr(dir));
			}
			if (directionLoc != -1) {
				glUniform3fv(diffuseLoc, 1, reinterpret_cast<const float*>(&light.diffuse));
			}

			GLenum err;
			while ((err = glGetError()) != GL_NO_ERROR) {
				std::cerr << "OpenGL error (during setting dir lights): " << err << std::endl;
			}
		}
		glUseProgram(0);
	}
}


void setLightUniforms(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<ProgramManager>& prgMngr) {
	setDirLightUniforms(registry, prgMngr);
}

void renderEntities(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<Camera>& camera) {
	auto view = registry->view<const comps::mesh, const comps::shaderProgram, const comps::transform>();
	for (auto [entity, mesh, prg, transform] : view.each()) {
		GLenum err;

		glUseProgram(prg.program);
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error (glUseProgram): " << err << std::endl;
		}
		glBindVertexArray(mesh.vao);
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error (glBindVertexArray): " << err << std::endl;
		}

		glUniformMatrix4fv(prg.modelUnifLoc, 1, GL_FALSE, glm::value_ptr(transform.matrix));
		glUniformMatrix4fv(prg.viewUnifLoc, 1, GL_FALSE, glm::value_ptr(camera->getView()));
		glUniformMatrix4fv(prg.projUnifLoc, 1, GL_FALSE, glm::value_ptr(camera->getProjection()));
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error (during setting matrices): " << err << std::endl;
		}

		if (prg.requireLights) {
			glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(camera->getView() * transform.matrix)));
			glUniformMatrix3fv(prg.normalUnifLoc, 1, GL_FALSE, glm::value_ptr(normalMat));
			while ((err = glGetError()) != GL_NO_ERROR) {
				std::cerr << "OpenGL error (during setting normal matrix): " << err << std::endl;
			}
		}

		if (registry->all_of<comps::material>(entity)) {
			const auto& color = registry->get<comps::material>(entity);
			glUniform3fv(glGetUniformLocation(prg.program, "aColor"), 1, reinterpret_cast<const float*>(&color.c));
			while ((err = glGetError()) != GL_NO_ERROR) {
				std::cerr << "OpenGL error (during setting aColor): " << err << std::endl;
			}
		}

		glDrawElements(GL_TRIANGLES, mesh.elementCount, mesh.indexType, 0);
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "OpenGL error (during render): " << err << std::endl;
		}

		glBindVertexArray(0);
		glUseProgram(0);
	}
}

void systems::render(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<Camera>& camera, const std::unique_ptr<ProgramManager>& prgMngr) {
	setLightUniforms(registry, prgMngr);
	renderEntities(registry, camera);
}