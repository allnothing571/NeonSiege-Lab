#include "sdl/SdlInput.h"

#include <SDL.h>

#include <algorithm>

namespace neon::sdl {

	SdlInput::SdlInput(
		SDL_Renderer* renderer)
		: renderer_(renderer) {
	}

	void SdlInput::handleEvent(
		const SDL_Event& event) {

		if (event.type == SDL_QUIT) {
			quitRequested_ = true;
			return;
		}

		if (event.type == SDL_MOUSEMOTION) {
			pendingUiCommand_.pointerMoved = true;
			pendingUiCommand_.pointerX = event.motion.x;
			pendingUiCommand_.pointerY = event.motion.y;
			return;
		}

		if (event.type == SDL_MOUSEBUTTONDOWN &&
			event.button.button == SDL_BUTTON_LEFT) {
			pendingUiCommand_.pointerPressed = true;
			pendingUiCommand_.pointerX = event.button.x;
			pendingUiCommand_.pointerY = event.button.y;
			return;
		}

		if (event.type != SDL_KEYDOWN ||
			event.key.repeat != 0) {
			return;
		}

		if (event.key.keysym.sym == SDLK_ESCAPE) {
			pausePressed_ = true;
			pendingUiCommand_.backPressed = true;
			return;
		}

		if (event.key.keysym.sym == SDLK_RETURN ||
			event.key.keysym.sym == SDLK_KP_ENTER) {
			pendingUiCommand_.confirmPressed = true;
			return;
		}

		if (event.key.keysym.sym == SDLK_UP ||
			event.key.keysym.sym == SDLK_w) {
			pendingUiCommand_.upPressed = true;
		}
		if (event.key.keysym.sym == SDLK_DOWN ||
			event.key.keysym.sym == SDLK_s) {
			pendingUiCommand_.downPressed = true;
		}
		if (event.key.keysym.sym == SDLK_LEFT ||
			event.key.keysym.sym == SDLK_a) {
			pendingUiCommand_.leftPressed = true;
		}
		if (event.key.keysym.sym == SDLK_RIGHT ||
			event.key.keysym.sym == SDLK_d) {
			pendingUiCommand_.rightPressed = true;
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

		int logicalMouseX = mouseX;
		int logicalMouseY = mouseY;
		mapPointerToLogical(
			mouseX,
			mouseY,
			logicalMouseX,
			logicalMouseY
		);

		command.aimPosition = {
			static_cast<float>(logicalMouseX),
			static_cast<float>(logicalMouseY)
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

	UiCommand SdlInput::consumeUiCommand() {
		UiCommand command = pendingUiCommand_;
		pendingUiCommand_ = UiCommand{};
		return command;
	}

	void SdlInput::mapPointerToLogical(
		int windowX,
		int windowY,
		int& logicalX,
		int& logicalY) const {
		logicalX = windowX;
		logicalY = windowY;
		if (renderer_ == nullptr) {
			return;
		}

		SDL_Window* window =
			SDL_RenderGetWindow(renderer_);
		int windowWidth = 0;
		int windowHeight = 0;
		int outputWidth = 0;
		int outputHeight = 0;
		int logicalWidth = 0;
		int logicalHeight = 0;
		if (window == nullptr ||
			SDL_GetRendererOutputSize(
				renderer_,
				&outputWidth,
				&outputHeight) != 0) {
			return;
		}

		SDL_GetWindowSize(
			window,
			&windowWidth,
			&windowHeight
		);
		SDL_RenderGetLogicalSize(
			renderer_,
			&logicalWidth,
			&logicalHeight
		);
		if (windowWidth <= 0 ||
			windowHeight <= 0 ||
			outputWidth <= 0 ||
			outputHeight <= 0 ||
			logicalWidth <= 0 ||
			logicalHeight <= 0) {
			return;
		}

		const float outputX =
			static_cast<float>(windowX) *
			static_cast<float>(outputWidth) /
			static_cast<float>(windowWidth);
		const float outputY =
			static_cast<float>(windowY) *
			static_cast<float>(outputHeight) /
			static_cast<float>(windowHeight);
		const float scale = std::min(
			static_cast<float>(outputWidth) /
				static_cast<float>(logicalWidth),
			static_cast<float>(outputHeight) /
				static_cast<float>(logicalHeight)
		);
		if (scale <= 0.0f) {
			return;
		}

		const float viewportX =
			(static_cast<float>(outputWidth) -
				static_cast<float>(logicalWidth) * scale) /
			2.0f;
		const float viewportY =
			(static_cast<float>(outputHeight) -
				static_cast<float>(logicalHeight) * scale) /
			2.0f;
		logicalX = static_cast<int>(
			(outputX - viewportX) / scale
		);
		logicalY = static_cast<int>(
			(outputY - viewportY) / scale
		);
	}

	bool SdlInput::quitRequested() const {
		return quitRequested_;
	}

}//namespace neon::sdl
