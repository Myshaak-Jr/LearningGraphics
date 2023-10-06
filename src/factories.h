#pragma once

#include <entt/entt.hpp>
#include "GLObjectManager.h"

namespace factories {
	void createCrane(const std::unique_ptr<entt::registry>& registry, const std::unique_ptr<GLObjectManager>& objMngr);
}