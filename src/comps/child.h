#pragma once

#include <entt/entt.hpp>


namespace comps {
	struct child {
		entt::entity parent;
		bool hasBeenCalculated;
	};
}