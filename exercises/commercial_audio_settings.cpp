#include "sdl/SdlAppSettings.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace {

	bool writeText(
		const std::filesystem::path& path,
		const std::string& text) {
		std::ofstream output(path);
		output << text;
		return static_cast<bool>(output);
	}

}

int main() {
	const std::filesystem::path root =
		std::filesystem::temp_directory_path() /
		"neon_audio_settings_test";
	std::error_code error{};
	std::filesystem::remove_all(root, error);
	std::filesystem::create_directories(root, error);
	if (error) {
		return 1;
	}
	const std::filesystem::path settingsPath =
		root / "settings.txt";

	if (!writeText(settingsPath, "1 2 1\n")) {
		return 1;
	}
	const neon::sdl::AppSettings legacy =
		neon::sdl::loadAppSettings(settingsPath.u8string());
	if (legacy.language != neon::sdl::UiLanguage::English ||
		legacy.windowSizeIndex != 2 ||
		legacy.windowMode !=
			neon::sdl::WindowMode::BorderlessFullscreen ||
		legacy.masterVolume != 80 ||
		legacy.effectsVolume != 100) {
		return 1;
	}

	neon::sdl::AppSettings saved{};
	saved.masterVolume = 40;
	saved.effectsVolume = 70;
	if (!neon::sdl::saveAppSettings(
		settingsPath.u8string(),
		saved)) {
		return 1;
	}
	const neon::sdl::AppSettings loaded =
		neon::sdl::loadAppSettings(settingsPath.u8string());
	if (loaded.masterVolume != 40 ||
		loaded.effectsVolume != 70) {
		return 1;
	}

	if (!writeText(settingsPath, "0 99 0 -25 250\n")) {
		return 1;
	}
	const neon::sdl::AppSettings clamped =
		neon::sdl::loadAppSettings(settingsPath.u8string());
	if (clamped.windowSizeIndex != 3 ||
		clamped.masterVolume != 0 ||
		clamped.effectsVolume != 100) {
		return 1;
	}

	if (!writeText(settingsPath, "damaged settings\n")) {
		return 1;
	}
	const neon::sdl::AppSettings fallback =
		neon::sdl::loadAppSettings(settingsPath.u8string());
	std::filesystem::remove_all(root, error);
	return fallback.masterVolume == 80 &&
		fallback.effectsVolume == 100
		? 0
		: 1;
}
