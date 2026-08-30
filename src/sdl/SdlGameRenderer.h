#pragma once

struct SDL_Renderer;

namespace neon {
	struct GameSnapshot;
}

namespace neon::sdl {
	class SdlGameRenderer {
	public:
		explicit SdlGameRenderer(SDL_Renderer* renderer);

		void render(
			const GameSnapshot& snapshot
		) const;

	private:
		SDL_Renderer* renderer_;
	};
}//namespace neon::sdl