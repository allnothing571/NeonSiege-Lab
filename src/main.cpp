#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <string_view>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include "GameState.h"
#include "SaveData.h"
#include "core/Collision.h"
#include "core/Player.h"
#include "core/Projectile.h"
#include "core/Enemy.h"
#include "core/WaveManager.h"
#include "sdl/SdlGameRenderer.h"
#include "sdl/SdlPaths.h"

int main(int argc, char* argv[]) {
	const bool smokeTest = argc > 1 && std::string_view(argv[1]) == "--smoke-test";

	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Neon Siege - Day 5",
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

	neon::sdl::SdlGameRenderer gameRenderer(renderer);

	const double frequency =
		static_cast<double>(SDL_GetPerformanceFrequency());
	Uint64 previousCounter = SDL_GetPerformanceCounter();

	const neon::Rect worldBounds{
		0.0f,
		0.0f,
		960.0f,
		540.0f
	};

	neon::Player player(
		neon::Vec2{ 456.0f, 246.0f },
		48.0f,
		240.0f,
		3
	);

	std::vector<neon::Projectile> bullets;
	std::vector<neon::Enemy> enemies;

	neon::WaveManager waveManager;
	waveManager.spawnNextWave(enemies);

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

	int score = 0;
	GameState gameState = GameState::Playing;

	constexpr double fixedDt = 1.0 / 60.0;
	double accumulator = 0.0;

	bool running = true;
	while (running) {
		//时间计算
		const Uint64 currentCounter = SDL_GetPerformanceCounter();
		constexpr double maxFrameTime = 0.25;

		const double rawFrameTime =
			(currentCounter - previousCounter) / frequency;

		const double frameTime =
			std::min(rawFrameTime, maxFrameTime);
		previousCounter = currentCounter;
		if (gameState == GameState::Playing) {
			accumulator += frameTime;
		}

		bool fireRequested = false;

		//事件循环
		SDL_Event event{};
		while (SDL_PollEvent(&event) != 0) {
			//退出
			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
			//射击
			if (event.type == SDL_MOUSEBUTTONDOWN &&
				event.button.button == SDL_BUTTON_LEFT) {
				fireRequested = true;
			}
			//暂停
			if (event.type == SDL_KEYDOWN &&
				event.key.keysym.sym == SDLK_p &&
				event.key.repeat == 0) {

				if (gameState == GameState::Playing) {
					gameState = GameState::Paused;
				}
				else if (gameState == GameState::Paused) {
					gameState = GameState::Playing;
				}
			}
			//重开
			if (event.type == SDL_KEYDOWN &&
				event.key.keysym.sym == SDLK_r &&
				event.key.repeat == 0 &&
				gameState == GameState::Gameover) {

				player.reset(neon::Vec2{ 456.0f, 246.0f }, 3);

				bullets.clear();
				enemies.clear();

				waveManager.reset();
				waveManager.spawnNextWave(enemies);

				score = 0;
				accumulator = 0.0;
				gameState = GameState::Playing;
			}
		}

		while (gameState == GameState::Playing && accumulator >= fixedDt) {
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

			player.update(
				neon::Vec2{ directionX,directionY },
				static_cast<float>(fixedDt),
				worldBounds);

			for (neon::Enemy& enemy : enemies) {
				enemy.update(
					player.center(),
					static_cast<float>(fixedDt)
				);

				if (neon::intersects(player.hitbox(), enemy.hitbox())) {
					player.takeDamage(1);
					enemy.defeat();
				}
			}

			if (!player.isAlive()) {
				gameState = GameState::Gameover;
			}

			for (neon::Projectile& bullet : bullets) {
				bullet.update(static_cast<float>(fixedDt));
			}

			for (neon::Projectile& bullet : bullets) {
				for (neon::Enemy& enemy : enemies) {
					if (!enemy.isAlive()) {
						continue;
					}

					if (neon::intersects(bullet.bounds(), enemy.bounds())) {
						enemy.takeDamage(10);
						bullet.consume();

						if (!enemy.isAlive()) {
							score += 100;

							if (score > highScore) {
								highScore = score;

								if (persistenceEnabled &&
									!saveData.saveHighScore(highScorePath, highScore)) {
									std::cerr << "无法保存最高分\n";
								}
							}
						}

						break;
					}
				}
			}

			enemies.erase(
				std::remove_if(
					enemies.begin(),
					enemies.end(),
					[](const neon::Enemy& enemy) {
						return !enemy.isAlive();
					}),
				enemies.end());

			if (enemies.empty() &&
				gameState == GameState::Playing) {
				waveManager.spawnNextWave(enemies);
			}

			bullets.erase(
				std::remove_if(
					bullets.begin(),
					bullets.end(),
					[&worldBounds](const neon::Projectile& bullet) {
						return bullet.isConsumed() ||
							bullet.isOutside(worldBounds);
					}),
				bullets.end());

			accumulator -= fixedDt;
		}

		int mouseX = 0;
		int mouseY = 0;
		SDL_GetMouseState(&mouseX, &mouseY);

		const neon::Vec2 playerCenter = player.center();
		const float playerCenterX = playerCenter.x;
		const float playerCenterY = playerCenter.y;

		float aimX = static_cast<float>(mouseX) - playerCenterX;
		float aimY = static_cast<float>(mouseY) - playerCenterY;

		const float aimLength =
			std::sqrt(aimX * aimX + aimY * aimY);

		if (aimLength > 0.0f) {
			aimX /= aimLength;
			aimY /= aimLength;
		}

		if (gameState == GameState::Playing && fireRequested && aimLength > 0.0f) {
			bullets.emplace_back(
				playerCenter,
				neon::Vec2{ aimX,aimY },
				8.0f,
				600.0f);
		}

		SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
		SDL_RenderClear(renderer);

		//绘制敌人
		for (const neon::Enemy& enemy : enemies) {
			gameRenderer.renderEnemy(enemy);
		}

		//绘制玩家
		gameRenderer.renderPlayer(player);

		//绘制子弹
		for (const neon::Projectile& bullet : bullets) {
			gameRenderer.renderProjectile(bullet);
		}

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

		//状态栏绘制
		const std::string title =
			"Neon Siege - Day 5 | Wave " +
			std::to_string(waveManager.currentWave()) +
			" | HP: " +
			std::to_string(player.health()) +
			" | Score: " +
			std::to_string(score) +
			" | High: " +
			std::to_string(highScore);

		SDL_SetWindowTitle(window, title.c_str());

		SDL_RenderPresent(renderer);

		if (smokeTest) {
			running = false;
		}
	}

	if (persistenceEnabled &&
		!saveData.saveHighScore(highScorePath, highScore)) {
		std::cerr << "无法保存最高分\n";
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
