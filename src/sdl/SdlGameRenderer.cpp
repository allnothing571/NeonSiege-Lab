#include "sdl/SdlGameRenderer.h"

#include <SDL.h>
#include <cmath>

#include "core/GameSnapshot.h"
#include "sdl/AssetManager.h"

namespace neon::sdl {

	SdlGameRenderer::SdlGameRenderer(
		SDL_Renderer* renderer,
		const AssetManager& assetManager)
		: renderer_(renderer),
		assetManager_(assetManager) {
	}

	void SdlGameRenderer::render(const GameSnapshot& snapshot)const {

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

		SDL_RenderPresent(renderer_);
	}

}//namespace neon::sdl

