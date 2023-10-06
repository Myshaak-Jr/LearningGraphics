#include "factories.h"

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

#include "comps/child.h"
#include "comps/position.h"
#include "comps/rotation.h"
#include "comps/scale.h"
#include "comps/transform.h"
#include "comps/rotatedByKeyboard.h"


void factories::createCrane(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<GLObjectManager>& objMngr) {
	const float speed = 10.0f;

	const float beltLen = 7.0f, beltWidth = 2.0f;
	const float upperArmLen = 9.5f, upperArmWidth = 2.2f;
	const float lowerArmLen = 5.0f, lowerArmWidth = 1.4f;
	const float palmWidth = 2.0f;
	const float fingerLen = 2.6f, fingerWidth = 0.5f;

	const float shoulderPitchMin = -90.0f, shoulderPitchMax = -20.0f;
	const float elbowPitchMin = 0.0f, elbowPitchMax = 146.25;
	const float wristPitchMin = -(elbowPitchMax - 90), wristPitchMax = 30.0f;
	const float wristRollMin = -90.0f, wristRollMax = 90.0f;
	const float fingerRollMin = -60.8f, fingerRollMax = -9.0f;

	auto crane = registry->create();
	registry->emplace<comps::position>(crane, glm::vec3(0.0f, -7.0f, -40.0f));
	registry->emplace<comps::rotation>(crane, -60.0f, 0.0f, 0.0f);
	registry->emplace<comps::scale>(crane);
	registry->emplace<comps::transform>(crane);
	registry->emplace<comps::rotatedByKeyboard<EAngle::YAW>>(crane, SDL_SCANCODE_A, SDL_SCANCODE_D, speed * 15.0f);

	auto leftBelt = registry->create();
	registry->emplace<comps::child>(leftBelt, crane);
	registry->emplace<comps::position>(leftBelt, glm::vec3((upperArmWidth + beltWidth) / 2.0f, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(leftBelt);
	registry->emplace<comps::scale>(leftBelt, glm::vec3(beltWidth, beltWidth, beltLen));
	registry->emplace<comps::transform>(leftBelt);
	registry->emplace<comps::mesh>(leftBelt, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(leftBelt, objMngr->getShaderProgram(0));

	auto rightBelt = registry->create();
	registry->emplace<comps::child>(rightBelt, crane);
	registry->emplace<comps::position>(rightBelt, glm::vec3(-(upperArmWidth + beltWidth) / 2.0f, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(rightBelt);
	registry->emplace<comps::scale>(rightBelt, glm::vec3(beltWidth, beltWidth, beltLen));
	registry->emplace<comps::transform>(rightBelt);
	registry->emplace<comps::mesh>(rightBelt, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(rightBelt, objMngr->getShaderProgram(0));

	auto shoulder = registry->create();
	registry->emplace<comps::child>(shoulder, crane);
	registry->emplace<comps::position>(shoulder, glm::vec3(0.0f, 0.0f, -1.0f));
	registry->emplace<comps::rotation>(shoulder, 0.0f, -60.7f, 0.0f);
	registry->emplace<comps::scale>(shoulder);
	registry->emplace<comps::transform>(shoulder);
	registry->emplace<comps::rotatedByKeyboard<EAngle::PITCH>>(shoulder, SDL_SCANCODE_S, SDL_SCANCODE_W, speed * 15.0f, shoulderPitchMin, shoulderPitchMax);

	auto upperArm = registry->create();
	registry->emplace<comps::child>(upperArm, shoulder);
	registry->emplace<comps::position>(upperArm, glm::vec3(0.0f, 0.0f, upperArmLen / 2));
	registry->emplace<comps::rotation>(upperArm);
	registry->emplace<comps::scale>(upperArm, glm::vec3(upperArmWidth, upperArmWidth, upperArmLen));
	registry->emplace<comps::transform>(upperArm);
	registry->emplace<comps::mesh>(upperArm, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(upperArm, objMngr->getShaderProgram(0));

	auto elbow = registry->create();
	registry->emplace<comps::child>(elbow, shoulder);
	registry->emplace<comps::position>(elbow, glm::vec3(0.0f, 0.0f, upperArmLen));
	registry->emplace<comps::rotation>(elbow, 0.0f, 76.1f, 0.0f);
	registry->emplace<comps::scale>(elbow);
	registry->emplace<comps::transform>(elbow);
	registry->emplace<comps::rotatedByKeyboard<EAngle::PITCH>>(elbow, SDL_SCANCODE_F, SDL_SCANCODE_R, speed * 15.0f, elbowPitchMin, elbowPitchMax);

	auto lowerArm = registry->create();
	registry->emplace<comps::child>(lowerArm, elbow);
	registry->emplace<comps::position>(lowerArm, glm::vec3(0.0f, 0.0f, lowerArmLen / 2.0f));
	registry->emplace<comps::rotation>(lowerArm);
	registry->emplace<comps::scale>(lowerArm, glm::vec3(lowerArmWidth, lowerArmWidth, lowerArmLen));
	registry->emplace<comps::transform>(lowerArm);
	registry->emplace<comps::mesh>(lowerArm, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(lowerArm, objMngr->getShaderProgram(0));

	auto wrist = registry->create();
	registry->emplace<comps::child>(wrist, elbow);
	registry->emplace<comps::position>(wrist, glm::vec3(0.0f, 0.0f, lowerArmLen));
	registry->emplace<comps::rotation>(wrist, 0.0f, 0.0f, 0.0f);
	registry->emplace<comps::scale>(wrist);
	registry->emplace<comps::transform>(wrist);
	registry->emplace<comps::rotatedByKeyboard<EAngle::PITCH>>(wrist, SDL_SCANCODE_G, SDL_SCANCODE_T, speed * 15.0f, wristPitchMin, wristPitchMax);
	registry->emplace<comps::rotatedByKeyboard<EAngle::ROLL>>(wrist, SDL_SCANCODE_C, SDL_SCANCODE_Z, speed * 15.0f, wristRollMin, wristRollMax);

	auto palm = registry->create();
	registry->emplace<comps::child>(palm, wrist);
	registry->emplace<comps::position>(palm, glm::vec3(0.0f, 0.0f, palmWidth / 2.0f));
	registry->emplace<comps::rotation>(palm);
	registry->emplace<comps::scale>(palm, glm::vec3(palmWidth, palmWidth, palmWidth));
	registry->emplace<comps::transform>(palm);
	registry->emplace<comps::mesh>(palm, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(palm, objMngr->getShaderProgram(0));

	auto leftUpperFingerJoint = registry->create();
	registry->emplace<comps::child>(leftUpperFingerJoint, wrist);
	registry->emplace<comps::position>(leftUpperFingerJoint, glm::vec3(palmWidth / 2.0f, -palmWidth / 2.0f, palmWidth / 2.0f));
	registry->emplace<comps::rotation>(leftUpperFingerJoint, 0.0f, 0.0f, -9.0f);
	registry->emplace<comps::scale>(leftUpperFingerJoint);
	registry->emplace<comps::transform>(leftUpperFingerJoint);
	registry->emplace<comps::rotatedByKeyboard<EAngle::ROLL>>(leftUpperFingerJoint, SDL_SCANCODE_E, SDL_SCANCODE_Q, speed * 15.0f, fingerRollMin, fingerRollMax);

	auto leftUpperFinger = registry->create();
	registry->emplace<comps::child>(leftUpperFinger, leftUpperFingerJoint);
	registry->emplace<comps::position>(leftUpperFinger, glm::vec3(fingerLen / 2.0f, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(leftUpperFinger);
	registry->emplace<comps::scale>(leftUpperFinger, glm::vec3(fingerLen, fingerWidth, fingerWidth));
	registry->emplace<comps::transform>(leftUpperFinger);
	registry->emplace<comps::mesh>(leftUpperFinger, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(leftUpperFinger, objMngr->getShaderProgram(0));

	auto leftLowerFingerJoint = registry->create();
	registry->emplace<comps::child>(leftLowerFingerJoint, leftUpperFingerJoint);
	registry->emplace<comps::position>(leftLowerFingerJoint, glm::vec3(fingerLen, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(leftLowerFingerJoint, 0.0f, 0.0f, -(9.0f + 30.0f));
	registry->emplace<comps::scale>(leftLowerFingerJoint);
	registry->emplace<comps::transform>(leftLowerFingerJoint);
	registry->emplace<comps::rotatedByKeyboard<EAngle::ROLL>>(leftLowerFingerJoint, SDL_SCANCODE_E, SDL_SCANCODE_Q, speed * 15.0f, fingerRollMin - 30.0f, fingerRollMax - 30.0f);

	auto leftLowerFinger = registry->create();
	registry->emplace<comps::child>(leftLowerFinger, leftLowerFingerJoint);
	registry->emplace<comps::position>(leftLowerFinger, glm::vec3(fingerLen / 2.0f, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(leftLowerFinger);
	registry->emplace<comps::scale>(leftLowerFinger, glm::vec3(fingerLen, fingerWidth, fingerWidth));
	registry->emplace<comps::transform>(leftLowerFinger);
	registry->emplace<comps::mesh>(leftLowerFinger, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(leftLowerFinger, objMngr->getShaderProgram(0));

	auto rightUpperFingerJoint = registry->create();
	registry->emplace<comps::child>(rightUpperFingerJoint, wrist);
	registry->emplace<comps::position>(rightUpperFingerJoint, glm::vec3(-palmWidth / 2.0f, -palmWidth / 2.0f, palmWidth / 2.0f));
	registry->emplace<comps::rotation>(rightUpperFingerJoint, 0.0f, 0.0f, +9.0f);
	registry->emplace<comps::scale>(rightUpperFingerJoint);
	registry->emplace<comps::transform>(rightUpperFingerJoint);
	registry->emplace<comps::rotatedByKeyboard<EAngle::ROLL>>(rightUpperFingerJoint, SDL_SCANCODE_Q, SDL_SCANCODE_E, speed * 15.0f, -fingerRollMax, -fingerRollMin);

	auto rightUpperFinger = registry->create();
	registry->emplace<comps::child>(rightUpperFinger, rightUpperFingerJoint);
	registry->emplace<comps::position>(rightUpperFinger, glm::vec3(-fingerLen / 2.0f, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(rightUpperFinger);
	registry->emplace<comps::scale>(rightUpperFinger, glm::vec3(fingerLen, fingerWidth, fingerWidth));
	registry->emplace<comps::transform>(rightUpperFinger);
	registry->emplace<comps::mesh>(rightUpperFinger, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(rightUpperFinger, objMngr->getShaderProgram(0));

	auto rightLowerFingerJoint = registry->create();
	registry->emplace<comps::child>(rightLowerFingerJoint, rightUpperFingerJoint);
	registry->emplace<comps::position>(rightLowerFingerJoint, glm::vec3(-fingerLen, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(rightLowerFingerJoint, 0.0f, 0.0f, +(9.0f + 30.0f));
	registry->emplace<comps::scale>(rightLowerFingerJoint);
	registry->emplace<comps::transform>(rightLowerFingerJoint);
	registry->emplace<comps::rotatedByKeyboard<EAngle::ROLL>>(rightLowerFingerJoint, SDL_SCANCODE_Q, SDL_SCANCODE_E, speed * 15.0f, -fingerRollMax + 30.0f, -fingerRollMin + 30.0f);

	auto rightLowerFinger = registry->create();
	registry->emplace<comps::child>(rightLowerFinger, rightLowerFingerJoint);
	registry->emplace<comps::position>(rightLowerFinger, glm::vec3(-fingerLen / 2.0f, 0.0f, 0.0f));
	registry->emplace<comps::rotation>(rightLowerFinger);
	registry->emplace<comps::scale>(rightLowerFinger, glm::vec3(fingerLen, fingerWidth, fingerWidth));
	registry->emplace<comps::transform>(rightLowerFinger);
	registry->emplace<comps::mesh>(rightLowerFinger, objMngr->getMesh(0));
	registry->emplace<comps::shaderProgram>(rightLowerFinger, objMngr->getShaderProgram(0));
}