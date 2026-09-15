#include "sdl/SdlGameRenderer.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include "core/GameSnapshot.h"
#include "sdl/AssetManager.h"

namespace neon::sdl {

	SdlGameRenderer::SdlGameRenderer(
		SDL_Renderer* renderer,
		const AssetManager& assetManager)
		: renderer_(renderer),
		assetManager_(assetManager),
		textRenderer_() {
	}

	void SdlGameRenderer::renderHud(
		const GameSnapshot& snapshot) const {

		int outputWidth = 0;
		int outputHeight = 0;
		if (SDL_GetRendererOutputSize(
			renderer_,
			&outputWidth,
			&outputHeight) != 0 ||
			outputWidth <= 0 ||
			outputHeight <= 0) {
			return;
		}

		const int barHeight = 48;
		SDL_BlendMode previousBlendMode =
			SDL_BLENDMODE_NONE;
		SDL_GetRenderDrawBlendMode(
			renderer_,
			&previousBlendMode
		);
		SDL_SetRenderDrawBlendMode(
			renderer_,
			SDL_BLENDMODE_BLEND
		);

		SDL_SetRenderDrawColor(
			renderer_,
			8,
			8,
			16,
			220
		);
		const SDL_Rect bar{
			0,
			0,
			outputWidth,
			barHeight
		};
		SDL_RenderFillRect(renderer_, &bar);

		const int maxHealth =
			std::max(1, snapshot.player.maxHealth);
		const int health =
			std::clamp(
				snapshot.player.health,
				0,
				maxHealth
			);
		const int healthWidth = 156;
		const SDL_Rect healthBackground{
			16,
			22,
			healthWidth,
			12
		};
		SDL_SetRenderDrawColor(
			renderer_,
			70,
			70,
			80,
			255
		);
		SDL_RenderFillRect(renderer_, &healthBackground);

		const SDL_Rect healthFill{
			16,
			22,
			healthWidth * health / maxHealth,
			12
		};
		SDL_SetRenderDrawColor(
			renderer_,
			health > maxHealth / 3 ? 60 : 240,
			health > maxHealth / 3 ? 220 : 70,
			90,
			255
		);
		SDL_RenderFillRect(renderer_, &healthFill);

		const bool chineseText =
			textRenderer_.supportsChinese();
		const std::string healthText =
			(chineseText ? "生命 " : "HP ") +
			std::to_string(health) +
			"/" +
			std::to_string(maxHealth);
		const std::string waveText =
			"Wave " +
			std::to_string(snapshot.currentWave) +
			"/" +
			std::to_string(std::max(1, snapshot.maximumWaves));
		std::string ammoText =
			(chineseText ? "弹药 " : "Ammo ") +
			std::to_string(snapshot.player.ammoInMagazine) +
			"/" +
			std::to_string(snapshot.player.magazineCapacity);
		if (snapshot.player.reloading) {
			ammoText +=
				"  Reload " +
				std::to_string(static_cast<int>(
					snapshot.player.reloadProgress * 100.0f
				)) +
				"%";
		}
		const std::string scoreText =
			"Score " + std::to_string(snapshot.score);

		const SDL_Color healthColor{ 240, 240, 250, 255 };
		const SDL_Color waveColor{ 150, 220, 255, 255 };
		const SDL_Color ammoColor{ 255, 220, 120, 255 };
		const SDL_Color scoreColor{ 220, 220, 230, 255 };

		textRenderer_.drawLeft(
			renderer_,
			healthText,
			TextStyle::Body,
			16,
			10,
			healthColor
		);
		textRenderer_.drawCentered(
			renderer_,
			waveText,
			TextStyle::Body,
			outputWidth / 2,
			24,
			waveColor
		);
		textRenderer_.drawRight(
			renderer_,
			ammoText,
			TextStyle::Body,
			outputWidth - 140,
			10,
			ammoColor
		);
		textRenderer_.drawRight(
			renderer_,
			scoreText,
			TextStyle::Body,
			outputWidth - 16,
			34,
			scoreColor
		);

		SDL_SetRenderDrawBlendMode(
			renderer_,
			previousBlendMode
		);
	}

	void SdlGameRenderer::renderStateOverlay(
		const GameSnapshot& snapshot) const {
		int outputWidth = 0;
		int outputHeight = 0;

		if (SDL_GetRendererOutputSize(
			renderer_,
			&outputWidth,
			&outputHeight) != 0 ||
			outputWidth <= 0 ||
			outputHeight <= 0) {
			return;
		}

		const int panelWidth =
			std::min(600, std::max(260, outputWidth - 40));
		const int panelHeight =
			std::min(190, std::max(150, outputHeight - 40));

		const SDL_Rect panel{
			(outputWidth - panelWidth) / 2,
			(outputHeight - panelHeight) / 2,
			panelWidth,
			panelHeight
		};

		SDL_BlendMode previousBlendMode =
			SDL_BLENDMODE_NONE;
		SDL_GetRenderDrawBlendMode(
			renderer_,
			&previousBlendMode
		);
		SDL_SetRenderDrawBlendMode(
			renderer_,
			SDL_BLENDMODE_BLEND
		);

		SDL_SetRenderDrawColor(
			renderer_,
			12,
			12,
			20,
			235
		);
		SDL_RenderFillRect(renderer_, &panel);

		SDL_SetRenderDrawColor(
			renderer_,
			255,
			70,
			100,
			255
		);
		SDL_RenderDrawRect(renderer_, &panel);

		const bool chineseText =
			textRenderer_.supportsChinese();
		std::string title;
		std::string prompt;
		if (snapshot.state == GameState::Paused) {
			title = chineseText ? "游戏已暂停" : "PAUSED";
			prompt = chineseText ? "按 P 继续" : "PRESS P TO RESUME";
		}
		else if (snapshot.state == GameState::Intermission) {
			title = chineseText
				? "第 " + std::to_string(snapshot.currentWave) + " 波完成"
				: "WAVE " + std::to_string(snapshot.currentWave) + " COMPLETE";
			std::ostringstream timer;
			timer << std::fixed << std::setprecision(1)
				<< std::max(0.0f, snapshot.intermissionRemaining);
			prompt = chineseText
				? "下一波将在 " + timer.str() + " 秒后开始"
				: "NEXT WAVE IN " + timer.str() + "s";
		}
		else if (snapshot.state == GameState::Victory) {
			title = chineseText ? "挑战完成" : "VICTORY";
			prompt = chineseText ? "按 R 键重新开始" : "PRESS R TO RESTART";
		}
		else {
			title = chineseText ? "玩家已死亡" : "GAME OVER";
			prompt = chineseText ? "按 R 键重新开始" : "PRESS R TO RESTART";
		}

		const SDL_Color titleColor{
			255,
			110,
			130,
			255
		};
		const SDL_Color promptColor{
			240,
			240,
			250,
			255
		};

		textRenderer_.drawCentered(
			renderer_,
			title,
			TextStyle::Title,
			panel.x + panel.w / 2,
			panel.y + panel.h / 2 - 34,
			titleColor
		);
		textRenderer_.drawCentered(
			renderer_,
			prompt,
			TextStyle::Body,
			panel.x + panel.w / 2,
			panel.y + panel.h / 2 + 34,
			promptColor
		);

		SDL_SetRenderDrawBlendMode(
			renderer_,
			previousBlendMode
		);
	}

	void SdlGameRenderer::render(
		const GameSnapshot& snapshot,
		const std::vector<PresentationEvent>& presentationEvents,
		float frameDt) {
		if (snapshot.state == GameState::Playing &&
			(lastRenderedState_ == GameState::Gameover ||
				lastRenderedState_ == GameState::Victory)) {
			presentationEffects_.reset();
		}

		presentationEffects_.update(
			frameDt,
			presentationEvents
		);

		const auto drawRectangle =
			[this](
				const Rect& bounds,
				Uint8 red,
				Uint8 green,
				Uint8 blue) {

					const SDL_FRect rectangle{
						bounds.x,
						bounds.y,
						bounds.w,
						bounds.h
					};

					SDL_SetRenderDrawColor(
						renderer_,
						red,
						green,
						blue,
						255
					);
					SDL_RenderFillRectF(
						renderer_,
						&rectangle
					);
			};

		const auto drawTextureOrRectangle =
			[this, &drawRectangle](
				TextureId textureId,
				const Rect& bounds,
				Uint8 red,
				Uint8 green,
				Uint8 blue) {

					SDL_Texture* texture =
						assetManager_.texture(textureId);

					if (texture != nullptr) {
						const SDL_FRect destination{
							bounds.x,
							bounds.y,
							bounds.w,
							bounds.h
						};

						if (SDL_RenderCopyF(
							renderer_,
							texture,
							nullptr,
							&destination) == 0) {
							return;
						}
					}

					drawRectangle(
						bounds,
						red,
						green,
						blue
					);
			};

		SDL_SetRenderDrawColor(
			renderer_,
			18,
			18,
			18,
			255
		);

		SDL_RenderClear(renderer_);

		for (const ObstacleSnapshot& obstacle :
			snapshot.obstacles) {

			drawTextureOrRectangle(
				TextureId::Obstacle,
				obstacle.bounds,
				100,
				100,
				120
			);
		}

		for (const EnemySnapshot& enemy :
			snapshot.enemies) {

			const bool isShooter =
				enemy.kind == EnemyKind::Shooter;

			const Uint8 shooterGreen =
				enemy.warningActive
				? static_cast<Uint8>(
					150.0f +
					105.0f * enemy.warningProgress
					)
				: 150;

			drawTextureOrRectangle(
				isShooter
				? TextureId::EnemyShooter
				: TextureId::EnemyChaser,
				enemy.bounds,
				isShooter ? 255 : 220,
				isShooter ? shooterGreen : 60,
				isShooter ? 40 : 70
			);
		}

		drawTextureOrRectangle(
			TextureId::Player,
			snapshot.player.bounds,
			snapshot.player.invulnerable ? 255 : 0,
			snapshot.player.invulnerable ? 255 : 220,
			255
		);

		for (const ProjectileSnapshot& projectile :
			snapshot.projectiles) {

			const bool isEnemyProjectile =
				projectile.faction ==
				ProjectileFaction::Enemy;

			drawTextureOrRectangle(
				isEnemyProjectile
				? TextureId::EnemyProjectile
				: TextureId::PlayerProjectile,
				projectile.bounds,
				isEnemyProjectile ? 255 : 255,
				isEnemyProjectile ? 70 : 220,
				isEnemyProjectile ? 180 : 80
			);
		}

		const Vec2 playerCenter{
			snapshot.player.bounds.x +
			snapshot.player.bounds.w / 2.0f,
			snapshot.player.bounds.y +
			snapshot.player.bounds.h / 2.0f
		};

		Vec2 aimDirection{
			snapshot.aimPosition.x -
			playerCenter.x,
			snapshot.aimPosition.y -
			playerCenter.y
		};

		const float aimLength =
			std::sqrt(
				aimDirection.x * aimDirection.x +
				aimDirection.y * aimDirection.y
			);

		if (aimLength > 0.0f) {
			aimDirection.x /= aimLength;
			aimDirection.y /= aimLength;

			constexpr float lineLength = 60.0f;

			SDL_SetRenderDrawColor(
				renderer_,
				255,
				80,
				160,
				255
			);

			SDL_RenderDrawLineF(
				renderer_,
				playerCenter.x,
				playerCenter.y,
				playerCenter.x +
				aimDirection.x * lineLength,
				playerCenter.y +
				aimDirection.y * lineLength
			);
		}

		presentationEffects_.render(
			renderer_,
			textRenderer_
		);

		renderHud(snapshot);

		if (snapshot.state != GameState::Playing) {
			renderStateOverlay(snapshot);
		}

		SDL_RenderPresent(renderer_);
		lastRenderedState_ = snapshot.state;
	}

}//namespace neon::sdl

