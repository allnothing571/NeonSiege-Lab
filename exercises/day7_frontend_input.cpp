#define SDL_MAIN_HANDLED

#include <SDL.h>

#include "sdl/SdlInput.h"

int main() {
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		return 1;
	}

	neon::sdl::SdlInput input{};
	SDL_Event event{};
	event.type = SDL_KEYDOWN;
	event.key.repeat = 0;

	event.key.keysym.sym = SDLK_ESCAPE;
	input.handleEvent(event);
	const neon::InputCommand escapeCommand =
		input.consumeCommand();
	const neon::sdl::UiCommand escapeUi =
		input.consumeUiCommand();
	if (!escapeCommand.pausePressed ||
		!escapeUi.backPressed ||
		input.quitRequested()) {
		SDL_Quit();
		return 1;
	}

	event.key.keysym.sym = SDLK_p;
	input.handleEvent(event);
	const neon::InputCommand oldPauseCommand =
		input.consumeCommand();
	input.consumeUiCommand();
	if (oldPauseCommand.pausePressed) {
		SDL_Quit();
		return 1;
	}

	event.key.keysym.sym = SDLK_RETURN;
	input.handleEvent(event);
	input.consumeCommand();
	const neon::sdl::UiCommand confirmUi =
		input.consumeUiCommand();
	if (!confirmUi.confirmPressed) {
		SDL_Quit();
		return 1;
	}

	event = SDL_Event{};
	event.type = SDL_MOUSEMOTION;
	event.motion.x = 320;
	event.motion.y = 180;
	input.handleEvent(event);
	input.consumeCommand();
	const neon::sdl::UiCommand pointerUi =
		input.consumeUiCommand();
	if (!pointerUi.pointerMoved ||
		pointerUi.pointerX != 320 ||
		pointerUi.pointerY != 180) {
		SDL_Quit();
		return 1;
	}

	event = SDL_Event{};
	event.type = SDL_QUIT;
	input.handleEvent(event);
	const bool quitPassed = input.quitRequested();
	SDL_Quit();
	return quitPassed ? 0 : 1;
}
