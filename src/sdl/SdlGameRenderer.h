#pragma once

#include <vector>

#include "core/GameState.h"
#include "core/PresentationEvent.h"
#include "sdl/SdlTextRenderer.h"
#include "sdl/SdlPresentationEffects.h"

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
			const GameSnapshot& snapshot,
			const std::vector<PresentationEvent>& presentationEvents,
			float frameDt
		);

	private:
		void renderHud(const GameSnapshot& snapshot) const;
		void renderStateOverlay(const GameSnapshot& snapshot) const;

		SDL_Renderer* renderer_;
		const AssetManager& assetManager_;
		SdlTextRenderer textRenderer_;
		SdlPresentationEffects presentationEffects_;
		GameState lastRenderedState_ = GameState::Playing;
	};
}//namespace neon::sdl
