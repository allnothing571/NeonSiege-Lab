#pragma once

#include <array>
#include <string>

struct SDL_Window;

namespace neon::sdl {

	enum class UiLanguage {
		Chinese,
		English
	};

	enum class WindowMode {
		Windowed,
		BorderlessFullscreen
	};

	struct WindowSizePreset {
		int width = 1280;
		int height = 720;
	};

	struct AppSettings {
		UiLanguage language = UiLanguage::Chinese;
		int windowSizeIndex = 1;
		WindowMode windowMode = WindowMode::Windowed;
		int masterVolume = 80;
		int effectsVolume = 100;
	};

	const std::array<WindowSizePreset, 4>&
		windowSizePresets();

	AppSettings loadAppSettings(
		const std::string& path
	);

	bool saveAppSettings(
		const std::string& path,
		const AppSettings& settings
	);

	bool applyAppSettings(
		SDL_Window* window,
		const AppSettings& settings
	);

}//namespace neon::sdl
