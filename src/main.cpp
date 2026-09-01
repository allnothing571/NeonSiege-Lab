#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>

#include "SaveData.h"
#include "core/GameSnapshot.h"
#include "core/GameplayConfig.h"
#include "core/InputCommand.h"
#include "core/Simulation.h"
#include "sdl/SdlGameRenderer.h"
#include "sdl/SdlInput.h"
#include "sdl/SdlPaths.h"
#include "sdl/AssetManager.h"

int main(int argc, char* argv[]) {
	const bool smokeTest = argc > 1 && std::string_view(argv[1]) == "--smoke-test";

	const neon::GameplayConfig config{};

	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Neon Siege - Day 5",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		static_cast<int>(config.worldBounds.w),
		static_cast<int>(config.worldBounds.h),
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

	neon::sdl::AssetManager assetManager;

	neon::sdl::SdlGameRenderer gameRenderer(
		renderer,
		assetManager
	);

	neon::sdl::SdlInput sdlInput;
	neon::Simulation simulation(config);

	const double frequency =
		static_cast<double>(SDL_GetPerformanceFrequency());
	Uint64 previousCounter = SDL_GetPerformanceCounter();

	//创建最高分
	SaveData saveData;

	const std::string highScorePath =
		smokeTest
		? std::string{}
	: neon::sdl::preferenceFilePath("high_score.txt");

	const bool persistenceEnabled =
		!highScorePath.empty();

	if (!smokeTest && !persistenceEnabled) {
		std::cerr << "无法获取存档路径\n";
	}

	int highScore =
		persistenceEnabled
		? saveData.loadHighScore(highScorePath)
		: 0;

	constexpr double fixedDt = 1.0 / 60.0;
	double accumulator = 0.0;

	bool pendingReloadPressed = false;

	bool running = true;

	while (running) {
		const Uint64 currentCounter =
			SDL_GetPerformanceCounter();

		constexpr double maxFrameTime = 0.25;

		const double rawFrameTime =
			(currentCounter - previousCounter) /
			frequency;

		const double frameTime =
			std::min(rawFrameTime, maxFrameTime);

		previousCounter = currentCounter;

		SDL_Event event{};

		while (SDL_PollEvent(&event) != 0) {
			sdlInput.handleEvent(event);
		}

		if (sdlInput.quitRequested()) {
			break;
		}

		neon::InputCommand command =
			sdlInput.consumeCommand();

		pendingReloadPressed =
			pendingReloadPressed ||
			command.reloadPressed;

		command.reloadPressed =
			pendingReloadPressed;

		const bool stateCommand =
			command.pausePressed ||
			(command.restartPressed &&
				simulation.state() ==
				neon::GameState::Gameover);

		if (stateCommand) {
			simulation.step(
				command,
				static_cast<float>(fixedDt)
			);

			pendingReloadPressed = false;

			command.pausePressed = false;
			command.restartPressed = false;
			command.reloadPressed = false;

			accumulator = 0.0;
		}
		else if (
			simulation.state() ==
			neon::GameState::Playing) {

			accumulator += frameTime;
		}

		while (simulation.state() ==
			neon::GameState::Playing &&
			accumulator >= fixedDt) {

			simulation.step(
				command,
				static_cast<float>(fixedDt)
			);

			pendingReloadPressed = false;

			//单次按键不能再补帧期间重复生效
			command.pausePressed = false;
			command.restartPressed = false;
			command.reloadPressed = false;

			accumulator -= fixedDt;
		}

		const neon::GameSnapshot snapshot =
			simulation.snapshot();

		if (snapshot.score > highScore) {
			highScore = snapshot.score;

			if (persistenceEnabled &&
				!saveData.saveHighScore(
					highScorePath,
					highScore)) {
				std::cerr << "无法保存最高分\n";
			}
		}

		const std::string title =
			"Neon Siege | Wave " +
			std::to_string(snapshot.currentWave) +
			" | HP: " +
			std::to_string(snapshot.player.health) +
			" | Ammo: " +
			std::to_string(snapshot.player.ammoInMagazine) +
			"/" +
			std::to_string(snapshot.player.magazineCapacity) +
			" | Score: " +
			std::to_string(snapshot.score) +
			" | High: " +
			std::to_string(highScore);

		SDL_SetWindowTitle(
			window,
			title.c_str()
		);

		gameRenderer.render(snapshot);

		if (smokeTest) {
			running = false;
		}
	}

	if (persistenceEnabled &&
		!saveData.saveHighScore(highScorePath, highScore)) {
		std::cerr << "无法保存最高分\n";
	}

	assetManager.clear();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
