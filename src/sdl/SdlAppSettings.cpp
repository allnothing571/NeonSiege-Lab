#include "sdl/SdlAppSettings.h"

#include <SDL.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace neon::sdl {

	namespace {

		AppSettings sanitized(
			AppSettings settings) {
			settings.windowSizeIndex = std::clamp(
				settings.windowSizeIndex,
				0,
				static_cast<int>(windowSizePresets().size()) - 1
			);
			return settings;
		}

	}//namespace

	const std::array<WindowSizePreset, 4>&
		windowSizePresets() {
		static const std::array<WindowSizePreset, 4> presets{
			WindowSizePreset{ 960, 540 },
			WindowSizePreset{ 1280, 720 },
			WindowSizePreset{ 1600, 900 },
			WindowSizePreset{ 1920, 1080 }
		};
		return presets;
	}

	AppSettings loadAppSettings(
		const std::string& path) {
		AppSettings settings{};
		if (path.empty()) {
			return settings;
		}

		std::ifstream input(std::filesystem::u8path(path));
		int language = 0;
		int windowMode = 0;
		if (!(input >> language >>
			settings.windowSizeIndex >>
			windowMode)) {
			return settings;
		}

		settings.language = language == 1
			? UiLanguage::English
			: UiLanguage::Chinese;
		settings.windowMode = windowMode == 1
			? WindowMode::BorderlessFullscreen
			: WindowMode::Windowed;
		return sanitized(settings);
	}

	bool saveAppSettings(
		const std::string& path,
		const AppSettings& settings) {
		if (path.empty()) {
			return false;
		}

		const AppSettings safeSettings =
			sanitized(settings);
		std::ofstream output(std::filesystem::u8path(path));
		if (!output) {
			return false;
		}

		output
			<< (safeSettings.language == UiLanguage::English ? 1 : 0)
			<< ' '
			<< safeSettings.windowSizeIndex
			<< ' '
			<< (safeSettings.windowMode ==
				WindowMode::BorderlessFullscreen ? 1 : 0)
			<< '\n';
		return static_cast<bool>(output);
	}

	bool applyAppSettings(
		SDL_Window* window,
		const AppSettings& settings) {
		if (window == nullptr) {
			return false;
		}

		const AppSettings safeSettings =
			sanitized(settings);
		if (safeSettings.windowMode ==
			WindowMode::BorderlessFullscreen) {
			return SDL_SetWindowFullscreen(
				window,
				SDL_WINDOW_FULLSCREEN_DESKTOP
			) == 0;
		}

		if (SDL_SetWindowFullscreen(window, 0) != 0) {
			return false;
		}

		const WindowSizePreset preset =
			windowSizePresets()[safeSettings.windowSizeIndex];
		SDL_SetWindowSize(window, preset.width, preset.height);
		SDL_SetWindowPosition(
			window,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED
		);
		return true;
	}

}//namespace neon::sdl
