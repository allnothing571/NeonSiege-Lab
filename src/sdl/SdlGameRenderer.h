#pragma once

#include <string_view>
#include <vector>

#include "core/GameState.h"
#include "core/PresentationEvent.h"
#include "sdl/SdlAppSettings.h"
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

		void setLanguage(
			UiLanguage language
		) noexcept;

		void setPauseMenuReturnEnabled(
			bool enabled
		) noexcept;

		void resetPresentationEffects();

		void renderMainMenu(
			int selectedItem
		) const;

		void renderHowToPlay(
			bool backHovered
		) const;

		void renderSettings(
			const AppSettings& settings,
			int selectedItem
		) const;

		int mainMenuItemAt(
			int x,
			int y
		) const;

		int settingsItemAt(
			int x,
			int y
		) const;

		bool howToPlayBackAt(
			int x,
			int y
		) const;

	private:
		bool chineseText() const noexcept;
		void renderFrontendBackground() const;
		void renderFrontendButton(
			std::string_view text,
			int centerY,
			int width,
			bool selected
		) const;
		void renderHud(const GameSnapshot& snapshot) const;
		void renderStateOverlay(const GameSnapshot& snapshot) const;

		SDL_Renderer* renderer_;
		const AssetManager& assetManager_;
		SdlTextRenderer textRenderer_;
		SdlPresentationEffects presentationEffects_;
		UiLanguage language_ = UiLanguage::Chinese;
		bool pauseMenuReturnEnabled_ = false;
		GameState lastRenderedState_ = GameState::Playing;
	};
}//namespace neon::sdl
