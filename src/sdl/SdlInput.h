#pragma once

#include "core/InputCommand.h"

union SDL_Event;

namespace neon::sdl {

	class SdlInput {
	public:
		void handleEvent(
			const SDL_Event& event
		);

		InputCommand consumeCommand();

		bool quitRequested() const;

	private:
		bool quitRequested_ = false;
		bool reloadPressed_ = false;
		bool pausePressed_ = false;
		bool restartPressed_ = false;
	};

}//namespace neon::sdl