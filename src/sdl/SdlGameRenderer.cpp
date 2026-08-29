#include "sdl/SdlGameRenderer.h"

#include <SDL.h>

#include "core/Player.h"
#include "core/Enemy.h"
#include "core/Projectile.h"

namespace neon::sdl {

	SdlGameRenderer::SdlGameRenderer(SDL_Renderer* renderer)
		: renderer_(renderer) {
	}

	void SdlGameRenderer::renderPlayer(const Player& player) const {
		const Rect playerBounds = player.bounds();

		SDL_FRect playerRect{
			playerBounds.x,
			playerBounds.y,
			playerBounds.w,
			playerBounds.h
		};

		SDL_SetRenderDrawColor(renderer_, 0, 220, 255, 255);
		SDL_RenderFillRectF(renderer_, &playerRect);
	}

	void SdlGameRenderer::renderEnemy(const Enemy& enemy)const {
		const Rect enemyBounds = enemy.bounds();

		SDL_FRect enemyRect{
			enemyBounds.x,
			enemyBounds.y,
			enemyBounds.w,
			enemyBounds.h
		};

		SDL_SetRenderDrawColor(renderer_, 220, 60, 70, 255);
		SDL_RenderFillRectF(renderer_, &enemyRect);
	}

	void SdlGameRenderer::renderProjectile(const Projectile& projectile)const {
		const Rect projectileBounds = projectile.bounds();

		SDL_FRect projectileRect{
			projectileBounds.x,
			projectileBounds.y,
			projectileBounds.w,
			projectileBounds.h
		};

		SDL_SetRenderDrawColor(renderer_, 255, 220, 80, 255);
		SDL_RenderFillRectF(renderer_, &projectileRect);
	}

}//namespace neon::sdl

