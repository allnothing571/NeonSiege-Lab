#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <string_view>
#include <cmath>
#include <algorithm>

int main(int argc, char* argv[]) {
	const bool smokeTest = argc > 1 && std::string_view(argv[1]) == "--smoke-test";

	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Neon Siege - Day 2",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		960,
		540,
		smokeTest ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);

	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == nullptr) {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	}

	if (renderer == nullptr) {
		std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	const double frequency =
		static_cast<double>(SDL_GetPerformanceFrequency());
	Uint64 previousCounter = SDL_GetPerformanceCounter();

	float playerX = 456.0f;
	float playerY = 246.0f;
	constexpr float playerSize = 48.0f;
	constexpr float playerSpeed = 240.0f;

	constexpr double fixedDt = 1.0 / 60.0;
	double accumulator = 0.0;

	bool running = true;
	while (running) {
		const Uint64 currentCounter = SDL_GetPerformanceCounter();
		constexpr double maxFrameTime = 0.25;

		const double rawFrameTime =
			(currentCounter - previousCounter) / frequency;

		const double frameTime =
			std::min(rawFrameTime, maxFrameTime);
		previousCounter = currentCounter;
		accumulator += frameTime;

		SDL_Event event{};
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
		}

		while (accumulator >= fixedDt) {
			const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

			float directionX = 0.0f;
			float directionY = 0.0f;

			if (keyboardState[SDL_SCANCODE_W]) {
				directionY -= 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_A]) {
				directionX -= 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_S]) {
				directionY += 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_D]) {
				directionX += 1.0f;
			}

			if (playerY <= 0 && directionY < 0.0f) {
				directionY = 0.0f;
			}
			if (playerY >= 540.0f - playerSize && directionY > 0.0f) {
				directionY = 0.0f;
			}
			if (playerX <= 0 && directionX < 0.0f) {
				directionX = 0.0f;
			}
			if (playerX >= 960.0f - playerSize && directionX > 0.0f) {
				directionX = 0.0f;
			}

			const float directionLength =
				std::sqrt(directionX * directionX + directionY * directionY);

			if (directionLength > 0.0f) {
				directionX /= directionLength;
				directionY /= directionLength;
			}

			playerX += directionX * playerSpeed * static_cast<float>(fixedDt);
			playerY += directionY * playerSpeed * static_cast<float>(fixedDt);

			playerX = std::clamp(playerX, 0.0f, 960.0f - playerSize);
			playerY = std::clamp(playerY, 0.0f, 540.0f - playerSize);

			accumulator -= fixedDt;
		}

		int mouseX = 0;
		int mouseY = 0;
		SDL_GetMouseState(&mouseX, &mouseY);

		const float playerCenterX = playerX + playerSize / 2.0f;
		const float playerCenterY = playerY + playerSize / 2.0f;

		float aimX = static_cast<float>(mouseX) - playerCenterX;
		float aimY = static_cast<float>(mouseY) - playerCenterY;

		const float aimLength =
			std::sqrt(aimX * aimX + aimY * aimY);

		if (aimLength > 0.0f) {
			aimX /= aimLength;
			aimY /= aimLength;
		}

		SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
		SDL_RenderClear(renderer);

		//绘制玩家
		SDL_FRect playerRect{
			playerX,
			playerY,
			playerSize,
			playerSize
		};

		//朝向线绘制
		constexpr float aimLineLength = 60.0f;

		SDL_SetRenderDrawColor(renderer, 255, 80, 160, 255);
		SDL_RenderDrawLineF(
			renderer,
			playerCenterX,
			playerCenterY,
			playerCenterX + aimX * aimLineLength,
			playerCenterY + aimY * aimLineLength
		);

		SDL_SetRenderDrawColor(renderer, 0, 220, 255, 255);
		SDL_RenderFillRectF(renderer, &playerRect);

		SDL_RenderPresent(renderer);

		if (smokeTest) {
			running = false;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
