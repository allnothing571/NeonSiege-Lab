#pragma once

#include <cstddef>
#include <vector>

#include "core/PresentationEvent.h"

struct SDL_Renderer;

namespace neon::sdl {

	class SdlTextRenderer;

	class SdlPresentationEffects {
	public:
		void reset();

		void update(
			float frameDt,
			const std::vector<PresentationEvent>& events
		);

		void render(
			SDL_Renderer* renderer,
			const SdlTextRenderer& textRenderer
		) const;

	private:
		struct TimedEffect {
			PresentationEventType type =
				PresentationEventType::None;
			Vec2 position{};
			int value = 0;
			float remaining = 0.0f;
			float duration = 0.0f;
		};

		static constexpr std::size_t maximumEffects = 32;

		float playerFlashRemaining_ = 0.0f;
		std::vector<TimedEffect> effects_{};
	};

}//namespace neon::sdl
