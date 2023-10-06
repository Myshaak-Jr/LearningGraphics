#pragma once

#include <SDL2/SDL.h>
#include "constants.h"


namespace comps {
	template <Axis A>
	struct movedByKeyboard {
		SDL_Scancode inc;
		SDL_Scancode dec;
		float speed;
	};
}