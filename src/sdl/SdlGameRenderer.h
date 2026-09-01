#pragma once

struct SDL_Renderer;

namespace neon {
	struct GameSnapshot;
}

namespace neon::sdl {
	class AssetManager;

	class SdlGameRenderer {
	public:
		SdlGameRenderer(SDL_Renderer* renderer,
			const AssetManager& assetManager
		);

		void render(
			const GameSnapshot& snapshot
		) const;

	private:
		SDL_Renderer* renderer_;
		const AssetManager& assetManager_;
	};
}//namespace neon::sdl