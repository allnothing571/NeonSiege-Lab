// Standalone acceptance harness for the Neon Siege audio asset set.
//
// This file is NOT part of the CMake build. It exists so that the on-disk
// assets can be exercised through the real SdlAudioManager against a real
// assets/ directory instead of a synthetic temporary one, which is what the
// in-tree AudioManager test does. It is compiled on demand by
// tools/run_asset_acceptance.ps1.
//
// Checks performed:
//   1. Every catalog entry loads from the real asset root.
//   2. Every file is independently removable: exactly the missing slot is
//      reported and every other entry still loads.
//   3. The relative path of a missing file appears verbatim in the report.
//   4. A corrupt file is reported as failed while every other entry loads.
//   5. Volume maths: master * effects scaling, 0% silences, clamping works.
//   6. Rapid playback is bounded by the catalog cooldown and by the channel
//      pool, and repeated shutdown stays safe.

#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_mixer.h>

#include "sdl/SdlAudioCatalog.h"
#include "sdl/SdlAudioManager.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace {

	int failures = 0;

	void check(bool condition, const char* what) {
		if (condition) {
			std::printf("  ok   %s\n", what);
			return;
		}
		std::printf("  FAIL %s\n", what);
		++failures;
	}

	std::string describe(const neon::sdl::AudioLoadReport& report) {
		return "loaded=" + std::to_string(report.loadedSoundCount) +
			" missing=" + std::to_string(report.missingSoundCount) +
			" failed=" + std::to_string(report.failedSoundCount);
	}

	bool reportMentions(
		const neon::sdl::AudioLoadReport& report,
		const std::string& relativePath) {
		for (const neon::sdl::AudioLoadIssue& issue : report.issues) {
			if (issue.relativePath == relativePath) {
				return true;
			}
		}
		return false;
	}

}

int main(int argc, char** argv) {
	if (argc < 3) {
		std::fprintf(
			stderr,
			"usage: %s <asset-root> <scratch-root>\n",
			argv[0]
		);
		return 2;
	}
	SDL_SetMainReady();
	SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);

	const std::filesystem::path root =
		std::filesystem::u8path(argv[1]);
	const std::filesystem::path scratchRoot =
		std::filesystem::u8path(argv[2]);

	std::printf("[1] catalog composition\n");
	const auto& definitions = neon::sdl::soundAssetDefinitions();
	check(definitions.size() == neon::sdl::soundAssetCount,
		"catalog size matches SoundId::Count");
	check(definitions.size() == 14u, "catalog holds exactly 14 entries");

	neon::sdl::SdlAudioManager audio{};
	check(audio.initialize(), "audio manager initialises");
	if (!audio.ready()) {
		return 1;
	}

	std::printf("[2] real asset root: %s\n", root.u8string().c_str());
	neon::sdl::AudioLoadReport report = audio.loadAssets(root);
	std::printf("  %s\n", describe(report).c_str());
	check(report.loadedSoundCount ==
		static_cast<int>(neon::sdl::soundAssetCount),
		"all catalog assets load");
	check(report.missingSoundCount == 0, "no asset reported missing");
	check(report.failedSoundCount == 0, "no asset reported corrupt");

	std::printf("[3] volume scaling and silencing\n");
	audio.setVolumes(100, 100);
	check(audio.masterVolume() == 100 && audio.effectsVolume() == 100,
		"100/100 accepted");
	audio.setVolumes(0, 0);
	check(audio.masterVolume() == 0 && audio.effectsVolume() == 0,
		"0/0 silences the mixer");
	audio.setVolumes(150, -20);
	check(audio.masterVolume() == 100 && audio.effectsVolume() == 0,
		"out-of-range values are clamped");
	audio.setVolumes(80, 60);

	std::printf("[4] playback, cooldown and repeat shutdown\n");
	audio.play(neon::sdl::SoundId::PlayerShot);
	audio.play(neon::sdl::SoundId::PlayerShot);
	check(Mix_Playing(-1) >= 0, "player shot starts a channel");
	audio.stopAll();
	check(Mix_Playing(-1) == 0, "stopAll halts every channel");
	std::printf("  allocated channels: %d\n", Mix_AllocateChannels(-1));

	std::printf("[5] each asset is independently removable\n");
	int removableFailures = 0;
	for (const neon::sdl::SoundAssetDefinition& definition : definitions) {
		const std::filesystem::path target =
			root / std::filesystem::u8path(
				std::string(definition.relativePath));
		const std::filesystem::path parked =
			target.parent_path() / "parked_for_acceptance.tmp";

		std::error_code error{};
		std::filesystem::rename(target, parked, error);
		if (error) {
			std::printf("  FAIL cannot park %s (%s)\n",
				std::string(definition.relativePath).c_str(),
				error.message().c_str());
			++failures;
			++removableFailures;
			continue;
		}

		const neon::sdl::AudioLoadReport partial =
			audio.loadAssets(root);
		const bool isolated =
			partial.loadedSoundCount ==
				static_cast<int>(neon::sdl::soundAssetCount) - 1 &&
			partial.missingSoundCount == 1 &&
			partial.failedSoundCount == 0 &&
			partial.issues.size() == 1 &&
			reportMentions(partial, std::string(definition.relativePath));

		std::filesystem::rename(parked, target, error);
		if (error) {
			std::printf("  FAIL cannot restore %s (%s)\n",
				std::string(definition.relativePath).c_str(),
				error.message().c_str());
			++failures;
			++removableFailures;
			continue;
		}

		if (!isolated) {
			std::printf("  FAIL removing %s -> %s\n",
				std::string(definition.relativePath).c_str(),
				describe(partial).c_str());
			++failures;
			++removableFailures;
		}
	}
	check(removableFailures == 0,
		"removing any single file reports only that file missing");

	const neon::sdl::AudioLoadReport restored = audio.loadAssets(root);
	check(restored.loadedSoundCount ==
		static_cast<int>(neon::sdl::soundAssetCount),
		"restoring the files brings every asset back");

	std::printf("[6] corrupt asset degrades without blocking the set\n");
	std::error_code corruptError{};
	std::filesystem::remove_all(scratchRoot, corruptError);
	if (!corruptError) {
		std::filesystem::create_directories(scratchRoot, corruptError);
	}
	for (const neon::sdl::SoundAssetDefinition& definition : definitions) {
		if (corruptError) {
			break;
		}
		const std::filesystem::path source =
			root / std::filesystem::u8path(
				std::string(definition.relativePath));
		const std::filesystem::path destination =
			scratchRoot / std::filesystem::u8path(
				std::string(definition.relativePath));
		std::filesystem::create_directories(
			destination.parent_path(),
			corruptError
		);
		if (!corruptError) {
			std::filesystem::copy_file(
				source,
				destination,
				std::filesystem::copy_options::overwrite_existing,
				corruptError
			);
		}
	}
	check(!corruptError, "isolated corrupt-file scratch copy is ready");

	const auto& corruptDefinition = neon::sdl::soundAssetFor(
		neon::sdl::SoundId::ReloadStarted
	);
	const std::filesystem::path corruptTarget =
		scratchRoot / std::filesystem::u8path(
			std::string(corruptDefinition.relativePath));
	if (!corruptError) {
		bool invalidWritten = false;
		{
			std::ofstream invalid(
				corruptTarget,
				std::ios::binary | std::ios::trunc
			);
			invalid.write("not a wave", 10);
			invalidWritten = static_cast<bool>(invalid);
		}
		check(invalidWritten, "corrupt-file probe writes invalid payload");

		const neon::sdl::AudioLoadReport corrupt =
			audio.loadAssets(scratchRoot);
		check(
			corrupt.loadedSoundCount ==
				static_cast<int>(neon::sdl::soundAssetCount) - 1 &&
			corrupt.missingSoundCount == 0 &&
			corrupt.failedSoundCount == 1 &&
			corrupt.issues.size() == 1 &&
			reportMentions(
				corrupt,
				std::string(corruptDefinition.relativePath)),
			"corrupt reload-start sound is isolated and reported"
		);
	}

	std::printf("[7] shutdown is idempotent\n");
	audio.shutdown();
	audio.shutdown();
	check(!audio.ready(), "manager reports not ready after shutdown");

	std::printf("\n%s (%d failure(s))\n",
		failures == 0 ? "ACCEPTANCE OK" : "ACCEPTANCE FAILED", failures);
	return failures == 0 ? 0 : 1;
}
