#define SDL_MAIN_HANDLED

#include <SDL.h>

#include "sdl/SdlAudioManager.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

	void write16(
		std::ofstream& output,
		std::uint16_t value) {
		const char bytes[]{
			static_cast<char>(value & 0xffu),
			static_cast<char>((value >> 8u) & 0xffu)
		};
		output.write(bytes, sizeof(bytes));
	}

	void write32(
		std::ofstream& output,
		std::uint32_t value) {
		const char bytes[]{
			static_cast<char>(value & 0xffu),
			static_cast<char>((value >> 8u) & 0xffu),
			static_cast<char>((value >> 16u) & 0xffu),
			static_cast<char>((value >> 24u) & 0xffu)
		};
		output.write(bytes, sizeof(bytes));
	}

	bool writeSilentWave(
		const std::filesystem::path& path) {
		std::error_code error{};
		std::filesystem::create_directories(
			path.parent_path(),
			error
		);
		if (error) {
			return false;
		}

		std::ofstream output(path, std::ios::binary);
		if (!output) {
			return false;
		}

		constexpr std::uint32_t sampleRate = 48000u;
		constexpr std::uint16_t channels = 1u;
		constexpr std::uint16_t bitsPerSample = 16u;
		constexpr std::uint32_t sampleCount = 32u;
		constexpr std::uint32_t dataSize =
			sampleCount * channels * bitsPerSample / 8u;

		output.write("RIFF", 4);
		write32(output, 36u + dataSize);
		output.write("WAVE", 4);
		output.write("fmt ", 4);
		write32(output, 16u);
		write16(output, 1u);
		write16(output, channels);
		write32(output, sampleRate);
		write32(output, sampleRate * channels * bitsPerSample / 8u);
		write16(output, channels * bitsPerSample / 8u);
		write16(output, bitsPerSample);
		output.write("data", 4);
		write32(output, dataSize);
		for (std::uint32_t index = 0;
			index < sampleCount;
			++index) {
			write16(output, 0u);
		}
		return static_cast<bool>(output);
	}

}

int main() {
	SDL_SetMainReady();
	SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);

	neon::sdl::SdlAudioManager audio{};
	if (!audio.initialize() || !audio.ready()) {
		return 1;
	}

	audio.setVolumes(-10, 150);
	if (audio.masterVolume() != 0 ||
		audio.effectsVolume() != 100) {
		return 1;
	}
	audio.setVolumes(80, 60);

	const std::filesystem::path root =
		std::filesystem::temp_directory_path() /
		"neon_audio_manager_test";
	std::error_code error{};
	std::filesystem::remove_all(root, error);
	const auto& firstDefinition =
		neon::sdl::soundAssetDefinitions().front();
	if (!writeSilentWave(
		root /
		std::filesystem::u8path(
			std::string(firstDefinition.relativePath)))) {
		return 1;
	}

	const neon::sdl::AudioLoadReport report =
		audio.loadAssets(root);
	if (report.loadedSoundCount != 1 ||
		report.missingSoundCount != 13 ||
		report.failedSoundCount != 0) {
		return 1;
	}

	audio.play(firstDefinition.id);
	audio.processPresentationEvents(
		std::vector<neon::PresentationEvent>{
			{ 0, neon::PresentationEventType::None },
			{ 0, neon::PresentationEventType::PlayerShot },
			{ 0, neon::PresentationEventType::EnemyShot },
			{ 0, neon::PresentationEventType::ProjectileHit },
			{ 0, neon::PresentationEventType::PlayerDamaged },
			{ 0, neon::PresentationEventType::EnemyDamaged },
			{ 0, neon::PresentationEventType::EnemyDied },
			{ 0, neon::PresentationEventType::ReloadStarted },
			{ 0, neon::PresentationEventType::ReloadCompleted },
			{ 0, neon::PresentationEventType::WaveStarted },
			{ 0, neon::PresentationEventType::UpgradeSelected },
			{ 0, neon::PresentationEventType::Victory },
			{ 0, neon::PresentationEventType::GameOver }
		}
	);
	audio.stopAll();
	audio.shutdown();
	audio.shutdown();
	std::filesystem::remove_all(root, error);
	SDL_Quit();
	return audio.ready() ? 1 : 0;
}
