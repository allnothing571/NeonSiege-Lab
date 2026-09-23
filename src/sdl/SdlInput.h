#pragma once

#include "core/InputCommand.h"

union SDL_Event;
struct SDL_Renderer;

namespace neon::sdl {

	struct UiCommand {
		bool confirmPressed = false;
		bool backPressed = false;
		bool upPressed = false;
		bool downPressed = false;
		bool leftPressed = false;
		bool rightPressed = false;
		bool pointerPressed = false;
		bool pointerMoved = false;
		int pointerX = 0;
		int pointerY = 0;
	};

	class SdlInput {
	public:
		explicit SdlInput(
			SDL_Renderer* renderer = nullptr
		);

		void handleEvent(
			const SDL_Event& event
		);

		InputCommand consumeCommand();
		UiCommand consumeUiCommand();

		bool quitRequested() const;

	private:
		void mapPointerToLogical(
			int windowX,
			int windowY,
			int& logicalX,
			int& logicalY
		) const;

		SDL_Renderer* renderer_ = nullptr;
		bool quitRequested_ = false;
		bool reloadPressed_ = false;
		bool pausePressed_ = false;
		bool restartPressed_ = false;
		UiCommand pendingUiCommand_{};
	};

}//namespace neon::sdl
