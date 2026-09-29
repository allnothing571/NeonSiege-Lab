#include "sdl/SdlAudioManager.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <algorithm>
#include <iostream>
#include <system_error>

#include "sdl/SdlPaths.h"

namespace neon::sdl {

	SdlAudioManager::~SdlAudioManager() {
		shutdown();
	}

	bool SdlAudioManager::initialize() {
		if (initialized_) {
			return true;
		}

		if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) == 0u) {
			if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
				std::cerr << "SDL audio initialization failed: "
					<< SDL_GetError() << '\n';
				return false;
			}
			ownsAudioSubsystem_ = true;
		}

		if (Mix_OpenAudio(
			48000,
			AUDIO_S16SYS,
			2,
			1024) != 0) {

			std::cerr << "SDL_mixer initialization failed: "
				<< Mix_GetError() << '\n';
			if (ownsAudioSubsystem_) {
				SDL_QuitSubSystem(SDL_INIT_AUDIO);
				ownsAudioSubsystem_ = false;
			}
			return false;
		}

		Mix_AllocateChannels(24);
		initialized_ = true;
		return true;
	}

	AudioLoadReport SdlAudioManager::loadAssets() {
		const std::string root = assetFilePath("");
		if (root.empty()) {
			AudioLoadReport report{};
			report.failedSoundCount =
				static_cast<int>(soundAssetCount);
			report.issues.push_back(
				AudioLoadIssue{
					{},
					"SDL_GetBasePath failed"
				}
			);
			return report;
		}
		return loadAssets(std::filesystem::u8path(root));
	}

	AudioLoadReport SdlAudioManager::loadAssets(
		const std::filesystem::path& assetRoot) {
		AudioLoadReport report{};
		clearChunks();

		if (!initialized_) {
			report.failedSoundCount =
				static_cast<int>(soundAssetCount);
			report.issues.push_back(
				AudioLoadIssue{
					{},
					"audio manager is not initialized"
				}
			);
			return report;
		}

		for (const SoundAssetDefinition& definition :
			soundAssetDefinitions()) {

			const std::string relativePath{
				definition.relativePath
			};
			const std::filesystem::path absolutePath =
				assetRoot /
				std::filesystem::u8path(relativePath);

			std::error_code error{};
			if (!std::filesystem::is_regular_file(
				absolutePath,
				error)) {

				++report.missingSoundCount;
				report.issues.push_back(
					AudioLoadIssue{
						relativePath,
						error
							? error.message()
							: "file is missing"
					}
				);
				continue;
			}

			const std::string utf8Path =
				absolutePath.u8string();
			Mix_Chunk* chunk = Mix_LoadWAV(
				utf8Path.c_str()
			);
			if (chunk == nullptr) {
				++report.failedSoundCount;
				report.issues.push_back(
					AudioLoadIssue{
						relativePath,
						Mix_GetError()
					}
				);
				continue;
			}

			chunks_[toIndex(definition.id)] = chunk;
			Mix_VolumeChunk(
				chunk,
				effectiveVolume(definition)
			);
			++report.loadedSoundCount;
		}

		return report;
	}

	void SdlAudioManager::setVolumes(
		int masterPercent,
		int effectsPercent) {

		masterVolumePercent_ = std::clamp(
			masterPercent,
			0,
			100
		);
		effectsVolumePercent_ = std::clamp(
			effectsPercent,
			0,
			100
		);
		applyChunkVolumes();
	}

	void SdlAudioManager::play(SoundId sound) {
		const std::size_t index = toIndex(sound);
		if (!initialized_ || index >= chunks_.size() ||
			chunks_[index] == nullptr) {
			return;
		}

		const SoundAssetDefinition& definition =
			soundAssetFor(sound);
		const std::uint64_t now = SDL_GetTicks64();
		if (hasPlayed_[index] &&
			now - lastPlayTicks_[index] <
				definition.cooldownMilliseconds) {
			return;
		}

		if (Mix_PlayChannel(-1, chunks_[index], 0) < 0) {
			if (!playFailureLogged_[index]) {
				std::cerr << "Unable to play sound "
					<< definition.relativePath << ": "
					<< Mix_GetError() << '\n';
				playFailureLogged_[index] = true;
			}
			return;
		}

		lastPlayTicks_[index] = now;
		hasPlayed_[index] = true;
	}

	void SdlAudioManager::processPresentationEvents(
		const std::vector<PresentationEvent>& events) {
		for (const PresentationEvent& event : events) {
			switch (event.type) {
			case PresentationEventType::PlayerShot:
				play(SoundId::PlayerShot);
				break;
			case PresentationEventType::EnemyShot:
				play(SoundId::EnemyShot);
				break;
			case PresentationEventType::ProjectileHit:
				play(SoundId::ProjectileHit);
				break;
			case PresentationEventType::PlayerDamaged:
				play(SoundId::PlayerDamaged);
				break;
			case PresentationEventType::EnemyDied:
				play(SoundId::EnemyDied);
				break;
			case PresentationEventType::ReloadStarted:
				play(SoundId::ReloadStarted);
				break;
			case PresentationEventType::ReloadCompleted:
				play(SoundId::ReloadCompleted);
				break;
			case PresentationEventType::WaveStarted:
				play(SoundId::WaveStarted);
				break;
			case PresentationEventType::UpgradeSelected:
				play(SoundId::UpgradeSelected);
				break;
			case PresentationEventType::Victory:
				play(SoundId::Victory);
				break;
			case PresentationEventType::GameOver:
				play(SoundId::GameOver);
				break;
			case PresentationEventType::None:
			case PresentationEventType::EnemyDamaged:
				break;
			}
		}
	}

	void SdlAudioManager::stopAll() {
		if (initialized_) {
			Mix_HaltChannel(-1);
		}
	}

	void SdlAudioManager::shutdown() {
		if (initialized_) {
			stopAll();
			clearChunks();
			Mix_CloseAudio();
			initialized_ = false;
		}

		if (ownsAudioSubsystem_) {
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
			ownsAudioSubsystem_ = false;
		}
	}

	bool SdlAudioManager::ready() const noexcept {
		return initialized_;
	}

	int SdlAudioManager::masterVolume() const noexcept {
		return masterVolumePercent_;
	}

	int SdlAudioManager::effectsVolume() const noexcept {
		return effectsVolumePercent_;
	}

	std::size_t SdlAudioManager::toIndex(
		SoundId sound) noexcept {
		return static_cast<std::size_t>(sound);
	}

	int SdlAudioManager::effectiveVolume(
		const SoundAssetDefinition& definition) const noexcept {
		const long long scaled =
			static_cast<long long>(MIX_MAX_VOLUME) *
			definition.baseVolumePercent *
			masterVolumePercent_ *
			effectsVolumePercent_;
		return static_cast<int>(std::clamp<long long>(
			scaled / 1000000LL,
			0,
			MIX_MAX_VOLUME
		));
	}

	void SdlAudioManager::applyChunkVolumes() {
		const auto& definitions = soundAssetDefinitions();
		for (std::size_t index = 0;
			index < chunks_.size();
			++index) {

			if (chunks_[index] != nullptr) {
				Mix_VolumeChunk(
					chunks_[index],
					effectiveVolume(definitions[index])
				);
			}
		}
	}

	void SdlAudioManager::clearChunks() {
		for (Mix_Chunk*& chunk : chunks_) {
			if (chunk != nullptr) {
				Mix_FreeChunk(chunk);
				chunk = nullptr;
			}
		}
		lastPlayTicks_.fill(0u);
		hasPlayed_.fill(false);
		playFailureLogged_.fill(false);
	}

}//namespace neon::sdl
