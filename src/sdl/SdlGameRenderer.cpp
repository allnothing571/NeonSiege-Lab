#include "sdl/SdlGameRenderer.h"

#include <SDL.h>
#include <cmath>

#include "core/GameSnapshot.h"

namespace neon::sdl {

	SdlGameRenderer::SdlGameRenderer(SDL_Renderer* renderer)
		: renderer_(renderer) {
	}

	void SdlGameRenderer::render(
		const GameSnapshot& snapshot)const {

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

			drawRectangle(
				obstacle.bounds,
				100,
				100,
				120
			);
		}

		for (const EnemySnapshot& enemy :
			snapshot.enemies) {

			drawRectangle(
				enemy.bounds,
				220,
				60,
				70
			);
		}

		drawRectangle(
			snapshot.player.bounds,
			0,
			220,
			255
		);

		for (const ProjectileSnapshot& projectile :
			snapshot.projectiles) {

			drawRectangle(
				projectile.bounds,
				255,
				220,
				80
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

