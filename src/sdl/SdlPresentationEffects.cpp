#include "sdl/SdlPresentationEffects.h"

#include <SDL.h>

#include <algorithm>
#include <string>

#include "sdl/SdlTextRenderer.h"

namespace neon::sdl {

	void SdlPresentationEffects::reset() {
		playerFlashRemaining_ = 0.0f;
		effects_.clear();
	}

	void SdlPresentationEffects::update(
		float frameDt,
		const std::vector<PresentationEvent>& events) {

		const float dt = std::max(0.0f, frameDt);
		playerFlashRemaining_ =
			std::max(0.0f, playerFlashRemaining_ - dt);

		for (TimedEffect& effect : effects_) {
			effect.remaining =
				std::max(0.0f, effect.remaining - dt);
		}

		effects_.erase(
			std::remove_if(
				effects_.begin(),
				effects_.end(),
				[](const TimedEffect& effect) {
					return effect.remaining <= 0.0f;
				}
			),
			effects_.end()
		);

		for (const PresentationEvent& event : events) {
			switch (event.type) {
			case PresentationEventType::PlayerDamaged:
				playerFlashRemaining_ =
					std::max(playerFlashRemaining_, 0.12f);
				break;

			case PresentationEventType::EnemyDamaged:
			case PresentationEventType::ProjectileHit:
				if (effects_.size() < maximumEffects) {
					effects_.push_back(
						TimedEffect{
							event.type,
							event.position,
							event.value,
							0.10f,
							0.10f
						}
					);
				}
				break;

			case PresentationEventType::EnemyDied:
				if (effects_.size() < maximumEffects) {
					effects_.push_back(
						TimedEffect{
							event.type,
							event.position,
							event.value,
							0.60f,
							0.60f
						}
					);
				}
				break;

			case PresentationEventType::None:
			case PresentationEventType::PlayerShot:
			case PresentationEventType::EnemyShot:
			case PresentationEventType::ReloadStarted:
			case PresentationEventType::ReloadCompleted:
			case PresentationEventType::WaveStarted:
			case PresentationEventType::UpgradeSelected:
			case PresentationEventType::Victory:
			case PresentationEventType::GameOver:
				break;
			}
		}
	}

	void SdlPresentationEffects::render(
		SDL_Renderer* renderer,
		const SdlTextRenderer& textRenderer) const {

		if (renderer == nullptr) {
			return;
		}

		SDL_BlendMode previousBlendMode =
			SDL_BLENDMODE_NONE;
		SDL_GetRenderDrawBlendMode(
			renderer,
			&previousBlendMode
		);
		SDL_SetRenderDrawBlendMode(
			renderer,
			SDL_BLENDMODE_BLEND
		);

		if (playerFlashRemaining_ > 0.0f) {
			const Uint8 alpha = static_cast<Uint8>(
				std::clamp(
					playerFlashRemaining_ / 0.12f * 110.0f,
					0.0f,
					110.0f
				)
			);

			int outputWidth = 0;
			int outputHeight = 0;
			if (SDL_GetRendererOutputSize(
				renderer,
				&outputWidth,
				&outputHeight) == 0) {

				const SDL_Rect screen{
					0,
					0,
					outputWidth,
					outputHeight
				};
				SDL_SetRenderDrawColor(
					renderer,
					255,
					40,
					60,
					alpha
				);
				SDL_RenderFillRect(renderer, &screen);
			}
		}

		for (const TimedEffect& effect : effects_) {
			const float progress =
				effect.duration > 0.0f
				? effect.remaining / effect.duration
				: 0.0f;

			if (effect.type == PresentationEventType::EnemyDied) {
				if (effect.value > 0 && textRenderer.ready()) {
					const SDL_Color color{
						255,
						230,
						90,
						static_cast<Uint8>(
							std::clamp(progress, 0.0f, 1.0f) * 255.0f
						)
					};
					textRenderer.drawCentered(
						renderer,
						"+" + std::to_string(effect.value),
						TextStyle::Body,
						static_cast<int>(effect.position.x),
						static_cast<int>(effect.position.y - 18.0f),
						color
					);
				}
				continue;
			}

			const float radius =
				8.0f + (1.0f - std::clamp(progress, 0.0f, 1.0f)) * 12.0f;
			const SDL_FRect ring{
				effect.position.x - radius,
				effect.position.y - radius,
				radius * 2.0f,
				radius * 2.0f
			};

			if (effect.type == PresentationEventType::EnemyDamaged) {
				SDL_SetRenderDrawColor(
					renderer,
					255,
					255,
					255,
					180
				);
			}
			else {
				SDL_SetRenderDrawColor(
					renderer,
					80,
					220,
					255,
					180
				);
			}

			SDL_RenderDrawRectF(renderer, &ring);
		}

		SDL_SetRenderDrawBlendMode(
			renderer,
			previousBlendMode
		);
	}

}//namespace neon::sdl
