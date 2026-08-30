#include "sdl/SdlInput.h"

#include <SDL.h>

namespace neon::sdl {

	void SdlInput::handleEvent(
		const SDL_Event& event) {

		if (event.type == SDL_QUIT) {
			quitRequested_ = true;
			return;
		}

		if (event.type != SDL_KEYDOWN ||
			event.key.repeat != 0) {
			return;
		}

		if (event.key.keysym.sym == SDLK_ESCAPE) {
			quitRequested_ = true;
			return;
		}

		if (event.key.keysym.sym == SDLK_p) {
			pausePressed_ = true;
		}

		if (event.key.keysym.sym == SDLK_r) {
			reloadPressed_ = true;
			restartPressed_ = true;
		}
	}

	InputCommand SdlInput::consumeCommand() {
		InputCommand command{};

		const Uint8* keyboardState =
			SDL_GetKeyboardState(nullptr);

		command.movement.x =
			(keyboardState[SDL_SCANCODE_D] ? 1.0f : 0.0f) -
			(keyboardState[SDL_SCANCODE_A] ? 1.0f : 0.0f);

		command.movement.y =
			(keyboardState[SDL_SCANCODE_S] ? 1.0f : 0.0f) -
			(keyboardState[SDL_SCANCODE_W] ? 1.0f : 0.0f);

		int mouseX = 0;
		int mouseY = 0;

		const Uint32 mouseButtons =
			SDL_GetMouseState(
				&mouseX,
				&mouseY
			);

		command.aimPosition = {
			static_cast<float>(mouseX),
			static_cast<float>(mouseY)
		};

		command.fireHeld =
			(mouseButtons &
				SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

		command.reloadPressed =
			reloadPressed_;

		command.pausePressed =
			pausePressed_;

		command.restartPressed =
			restartPressed_;

		reloadPressed_ = false;
		pausePressed_ = false;
		restartPressed_ = false;

		return command;
	}

	bool SdlInput::quitRequested() const {
		return quitRequested_;
	}

}//namespace neon::sdl