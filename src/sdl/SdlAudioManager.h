#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "core/PresentationEvent.h"
#include "sdl/SdlAudioCatalog.h"

struct Mix_Chunk;

namespace neon::sdl {

	struct AudioLoadIssue {
		std::string relativePath{};
		std::string message{};
	};

	struct AudioLoadReport {
		int loadedSoundCount = 0;
		int missingSoundCount = 0;
		int failedSoundCount = 0;
		std::vector<AudioLoadIssue> issues{};
	};

	class SdlAudioManager {
	public:
		SdlAudioManager() = default;
		~SdlAudioManager();

		SdlAudioManager(const SdlAudioManager&) = delete;
		SdlAudioManager& operator=(
			const SdlAudioManager&) = delete;

		bool initialize();
		AudioLoadReport loadAssets();
		AudioLoadReport loadAssets(
			const std::filesystem::path& assetRoot
		);
		void setVolumes(
			int masterPercent,
			int effectsPercent
		);
		void play(SoundId sound);
		void processPresentationEvents(
			const std::vector<PresentationEvent>& events
		);
		void stopAll();
		void shutdown();

		bool ready() const noexcept;
		int masterVolume() const noexcept;
		int effectsVolume() const noexcept;

	private:
		static std::size_t toIndex(SoundId sound) noexcept;
		int effectiveVolume(
			const SoundAssetDefinition& definition
		) const noexcept;
		void applyChunkVolumes();
		void clearChunks();

		std::array<Mix_Chunk*, soundAssetCount> chunks_{};
		std::array<std::uint64_t, soundAssetCount>
			lastPlayTicks_{};
		std::array<bool, soundAssetCount> hasPlayed_{};
		std::array<bool, soundAssetCount> playFailureLogged_{};
		int masterVolumePercent_ = 80;
		int effectsVolumePercent_ = 100;
		bool initialized_ = false;
		bool ownsAudioSubsystem_ = false;
	};

}//namespace neon::sdl
