#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "core/ReplayTape.h"
#include "core/Simulation.h"

namespace {

	bool nearlyEqual(
		float first,
		float second
	) {
		return std::fabs(first - second) < 0.00001f;
	}

	bool sameRect(
		const neon::Rect& first,
		const neon::Rect& second
	) {
		return
			nearlyEqual(first.x, second.x) &&
			nearlyEqual(first.y, second.y) &&
			nearlyEqual(first.w, second.w) &&
			nearlyEqual(first.h, second.h);
	}

	bool sameSnapshot(
		const neon::GameSnapshot& first,
		const neon::GameSnapshot& second
	) {
		if (first.state != second.state ||
			first.mapId != second.mapId ||
			first.score != second.score ||
			first.currentWave != second.currentWave ||
			!nearlyEqual(
				first.aimPosition.x,
				second.aimPosition.x
			) ||
			!nearlyEqual(
				first.aimPosition.y,
				second.aimPosition.y
			)) {
			return false;
		}

		if (!sameRect(
			first.player.bounds,
			second.player.bounds
		)) {
			return false;
		}

		if (first.player.health != second.player.health ||
			first.player.alive != second.player.alive ||
			first.player.invulnerable !=
			second.player.invulnerable ||
			first.player.ammoInMagazine !=
			second.player.ammoInMagazine ||
			first.player.magazineCapacity !=
			second.player.magazineCapacity ||
			first.player.reloading !=
			second.player.reloading ||
			!nearlyEqual(
				first.player.reloadProgress,
				second.player.reloadProgress
			)) {
			return false;
		}

		if (first.enemies.size() != second.enemies.size() ||
			first.projectiles.size() !=
			second.projectiles.size() ||
			first.obstacles.size() !=
			second.obstacles.size()) {
			return false;
		}

		for (std::size_t index = 0;
			index < first.enemies.size();
			++index) {
			const auto& firstEnemy =
				first.enemies[index];

			const auto& secondEnemy =
				second.enemies[index];

			if (!sameRect(
				firstEnemy.bounds,
				secondEnemy.bounds
			) ||
				firstEnemy.kind != secondEnemy.kind ||
				firstEnemy.alive != secondEnemy.alive ||
				firstEnemy.warningActive !=
				secondEnemy.warningActive ||
				!nearlyEqual(
					firstEnemy.warningProgress,
					secondEnemy.warningProgress
				)) {
				return false;
			}
		}

		for (std::size_t index = 0;
			index < first.projectiles.size();
			++index) {
			const auto& firstProjectile =
				first.projectiles[index];

			const auto& secondProjectile =
				second.projectiles[index];

			if (!sameRect(
				firstProjectile.bounds,
				secondProjectile.bounds
			) ||
				firstProjectile.faction !=
				secondProjectile.faction) {
				return false;
			}
		}

		for (std::size_t index = 0;
			index < first.obstacles.size();
			++index) {
			if (!sameRect(
				first.obstacles[index].bounds,
				second.obstacles[index].bounds
			)) {
				return false;
			}
		}

		return true;
	}

}//namespace

int main() {
	neon::GameplayConfig config{};
	config.randomSeed = 20260902u;

	constexpr float fixedDt = 1.0f / 60.0f;

	neon::Simulation source(config);
	if (!source.reset(neon::MapId::SplitCorridors)) {
		return 1;
	}
	source.consumeEvents();

	neon::ReplayTape tape{};
	tape.randomSeed = config.randomSeed;
	tape.mapId = source.snapshot().mapId;
	tape.fixedDt = fixedDt;

	std::vector<neon::GameSnapshot> expectedSnapshots{};

	for (int index = 0; index < 24; ++index) {
		neon::InputCommand command{};

		command.movement = {
			index % 2 == 0 ? 1.0f : -1.0f,
			index % 3 == 0 ? 0.5f : 0.0f
		};

		command.aimPosition = {
			700.0f + static_cast<float>(index),
			270.0f
		};

		command.fireHeld = index % 4 == 0;
		if (index == 20) {
			command.upgradeSelection = 2;
		}

		if (index == 8 ||
			index == 9) {
			command.pausePressed = true;
		}

		source.step(command, fixedDt);

		const auto recordedFrames =
			source.consumeRecordedInputs();

		if (recordedFrames.size() != 1) {
			return 1;
		}

		tape.frames.push_back(
			recordedFrames.front()
		);

		expectedSnapshots.push_back(
			source.snapshot()
		);
	}

	std::stringstream encoded;
	std::string error;

	if (!neon::writeReplay(
		encoded,
		tape,
		error
	)) {
		return 2;
	}

	encoded.seekg(0);

	neon::ReplayTape loaded{};

	if (!neon::readReplay(
		encoded,
		loaded,
		error
	)) {
		return 3;
	}

	if (loaded.version != tape.version ||
		loaded.randomSeed != tape.randomSeed ||
		loaded.mapId != tape.mapId ||
		!nearlyEqual(
			loaded.fixedDt,
			tape.fixedDt
		) ||
		loaded.frames.size() !=
			tape.frames.size() ||
		loaded.frames[20].command.upgradeSelection != 2) {
		return 4;
	}

	neon::GameplayConfig replayConfig = config;
	replayConfig.randomSeed = loaded.randomSeed;

	neon::Simulation replay(replayConfig);
	if (loaded.mapId != neon::MapId::Legacy &&
		!replay.reset(loaded.mapId)) {
		return 5;
	}
	replay.consumeEvents();

	for (std::size_t index = 0;
		index < loaded.frames.size();
		++index) {
		replay.step(
			loaded.frames[index].command,
			loaded.fixedDt
		);

		if (replay.tick() !=
			loaded.frames[index].tick ||
			!sameSnapshot(
				replay.snapshot(),
				expectedSnapshots[index]
			)) {
			return 5;
		}
	}

	std::string corruptedText =
		encoded.str();

	const std::size_t frameMarker =
		corruptedText.find("FRAME ");

	if (frameMarker == std::string::npos) {
		return 6;
	}

	corruptedText.replace(
		frameMarker,
		5,
		"FRAMX"
	);

	std::stringstream corruptedInput(
		corruptedText
	);

	neon::ReplayTape rejected{};

	if (neon::readReplay(
		corruptedInput,
		rejected,
		error
	)) {
		return 7;
	}

	std::stringstream legacyInput{
		"NEON_REPLAY 1\n"
		"SEED 1337\n"
		"FIXED_DT 0.0166666675\n"
		"FRAMES 0\n"
		"END\n"
	};
	neon::ReplayTape legacyTape{};

	if (!neon::readReplay(
		legacyInput,
		legacyTape,
		error
	) ||
		legacyTape.version != 1u ||
		legacyTape.mapId != neon::MapId::Legacy) {
		return 8;
	}

	std::stringstream versionTwoInput{
		"NEON_REPLAY 2\n"
		"SEED 1337\n"
		"MAP 1\n"
		"FIXED_DT 0.0166666675\n"
		"FRAMES 1\n"
		"FRAME 1 0 0 100 100 0 0 0 0\n"
		"END\n"
	};
	neon::ReplayTape versionTwoTape{};
	if (!neon::readReplay(
		versionTwoInput,
		versionTwoTape,
		error
	) ||
		versionTwoTape.version != 2u ||
		versionTwoTape.frames.size() != 1 ||
		versionTwoTape.frames[0].command.upgradeSelection != -1) {
		return 9;
	}

	return 0;
}
