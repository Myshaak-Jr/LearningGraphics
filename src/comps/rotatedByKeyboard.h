#pragma once

#include <SDL2/SDL.h>
#include "constants.h"


namespace comps {
	template <EAngle A>
	struct rotatedByKeyboard {
		SDL_Scancode inc;
		SDL_Scancode dec;
		float speed;
		bool clamped;
		float minVal = 0.0f;
		float maxVal = 0.0f;

		inline rotatedByKeyboard(
			SDL_Scancode inc, SDL_Scancode dec,
			float speed,
			bool clamped,
			float minVal, float maxVal
		)
			: inc(inc), dec(dec)
			, speed(speed)
			, clamped(clamped)
			, minVal(minVal), maxVal(maxVal)
		{}

		inline rotatedByKeyboard(SDL_Scancode inc, SDL_Scancode dec, float speed)
			: inc(inc), dec(dec)
			, speed(speed)
			, clamped(false)
			, minVal(0.0f), maxVal(0.0f)
		{}

		inline rotatedByKeyboard(SDL_Scancode inc, SDL_Scancode dec, float speed, float minVal, float maxVal)
			: inc(inc), dec(dec)
			, speed(speed)
			, clamped(true)
			, minVal(minVal), maxVal(maxVal)
		{}
	};
}