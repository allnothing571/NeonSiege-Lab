#pragma once

struct SDL_Renderer;

namespace neon {
	class Player;
	class Enemy;
	class Projectile;
}

namespace neon::sdl {
	class SdlGameRenderer {
	public:
		explicit SdlGameRenderer(SDL_Renderer* renderer);

		void renderPlayer(const Player& player) const;

		void renderEnemy(const Enemy& enemy) const;

		void renderProjectile(const Projectile& projectile) const;

	private:
		SDL_Renderer* renderer_;
	};
}//namespace neon::sdl