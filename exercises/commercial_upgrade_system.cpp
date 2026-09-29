#include <cmath>
#include <random>

#include "core/GameEvent.h"
#include "core/Simulation.h"
#include "core/UpgradeSystem.h"

namespace {

	constexpr float fixedDt = 1.0f / 60.0f;

	bool nearlyEqual(
		float first,
		float second) {
		return std::fabs(first - second) < 0.0001f;
	}

	bool sameStats(
		const neon::UpgradeStats& first,
		const neon::UpgradeStats& second) {
		return first.projectileDamage ==
				second.projectileDamage &&
			first.magazineCapacity ==
				second.magazineCapacity &&
			first.maximumHealth ==
				second.maximumHealth &&
			nearlyEqual(
				first.fireInterval,
				second.fireInterval) &&
			nearlyEqual(
				first.reloadDuration,
				second.reloadDuration) &&
			nearlyEqual(
				first.projectileSpreadDegrees,
				second.projectileSpreadDegrees) &&
			nearlyEqual(
				first.projectileSpeed,
				second.projectileSpeed);
	}

	bool runUntil(
		neon::Simulation& simulation,
		neon::GameState targetState,
		int minimumWave) {
		for (int stepIndex = 0;
			stepIndex < 8000;
			++stepIndex) {
			const neon::GameSnapshot snapshot =
				simulation.snapshot();
			if (snapshot.state == targetState &&
				snapshot.currentWave >= minimumWave) {
				return true;
			}

			neon::InputCommand command{};
			if (snapshot.state ==
				neon::GameState::Playing) {
				command.movement = neon::Vec2{
					stepIndex % 120 < 60 ? 1.0f : -1.0f,
					stepIndex % 180 < 90 ? 1.0f : -1.0f
				};
			}
			simulation.step(command, fixedDt);
		}
		return false;
	}

}//namespace

int main() {
	neon::GameplayConfig config{};
	neon::UpgradeSystem firstSystem{};
	neon::UpgradeSystem secondSystem{};
	std::mt19937 firstRandom{ 20260926u };
	std::mt19937 secondRandom{ 20260926u };

	if (!firstSystem.generateOffers(firstRandom) ||
		!secondSystem.generateOffers(secondRandom) ||
		firstSystem.offerCount() != 3 ||
		secondSystem.offerCount() != 3) {
		return 1;
	}

	bool foundOffense = false;
	bool foundSupport = false;
	for (int index = 0; index < 3; ++index) {
		const neon::UpgradeType first =
			firstSystem.offerAt(index);
		if (first != secondSystem.offerAt(index)) {
			return 2;
		}
		for (int earlier = 0; earlier < index; ++earlier) {
			if (first == firstSystem.offerAt(earlier)) {
				return 3;
			}
		}
		foundOffense =
			foundOffense ||
			neon::UpgradeSystem::offensive(first);
		foundSupport =
			foundSupport ||
			!neon::UpgradeSystem::offensive(first);
	}
	if (!foundOffense || !foundSupport) {
		return 4;
	}

	neon::UpgradeSystem exhaustedOffenseSystem{};
	std::mt19937 exhaustionRandom{ 77u };
	for (int selectionIndex = 0;
		selectionIndex < 3;
		++selectionIndex) {
		if (!exhaustedOffenseSystem.generateOffers(
			exhaustionRandom)) {
			return 4;
		}
		int offensiveOption = -1;
		for (int optionIndex = 0;
			optionIndex < 3;
			++optionIndex) {
			if (neon::UpgradeSystem::offensive(
				exhaustedOffenseSystem.offerAt(optionIndex))) {
				offensiveOption = optionIndex;
				break;
			}
		}
		if (offensiveOption < 0 ||
			!exhaustedOffenseSystem.applyOffer(
				offensiveOption)) {
			return 4;
		}
	}
	if (!exhaustedOffenseSystem.generateOffers(
		exhaustionRandom) ||
		exhaustedOffenseSystem.offerCount() != 3) {
		return 4;
	}
	for (int optionIndex = 0;
		optionIndex < 3;
		++optionIndex) {
		if (neon::UpgradeSystem::offensive(
			exhaustedOffenseSystem.offerAt(optionIndex))) {
			return 4;
		}
	}

	const neon::UpgradeStats baseStats =
		firstSystem.stats(config);
	if (baseStats.projectileDamage != 10 ||
		baseStats.magazineCapacity != 12 ||
		baseStats.maximumHealth != 100 ||
		!nearlyEqual(baseStats.fireInterval, 0.2f) ||
		!nearlyEqual(baseStats.reloadDuration, 1.2f) ||
		!nearlyEqual(
			baseStats.projectileSpreadDegrees,
			3.0f) ||
		!nearlyEqual(baseStats.projectileSpeed, 600.0f)) {
		return 5;
	}

	const neon::UpgradeStats fireRateStats =
		firstSystem.statsAfter(
			config,
			neon::UpgradeType::FireRate
		);
	const neon::UpgradeStats ammoStats =
		firstSystem.statsAfter(
			config,
			neon::UpgradeType::AmmoSystem
		);
	const neon::UpgradeStats armorStats =
		firstSystem.statsAfter(
			config,
			neon::UpgradeType::ArmorCore
		);
	const neon::UpgradeStats ballisticStats =
		firstSystem.statsAfter(
			config,
			neon::UpgradeType::BallisticCalibration
		);
	if (!nearlyEqual(
			fireRateStats.fireInterval,
			0.2f / 1.1f) ||
		ammoStats.magazineCapacity != 15 ||
		!nearlyEqual(ammoStats.reloadDuration, 1.1f) ||
		armorStats.maximumHealth != 115 ||
		!nearlyEqual(
			ballisticStats.projectileSpreadDegrees,
			2.0f) ||
		!nearlyEqual(
			ballisticStats.projectileSpeed,
			660.0f)) {
		return 6;
	}

	neon::GameplayConfig flowConfig{};
	flowConfig.worldBounds =
		neon::Rect{ 0.0f, 0.0f, 200.0f, 200.0f };
	flowConfig.playerStartPosition =
		neon::Vec2{ 76.0f, 76.0f };
	flowConfig.maximumWaves = 3;
	flowConfig.maximumEnemiesPerWave = 1;
	flowConfig.waveIntermissionDuration = 0.1f;
	flowConfig.enemyBaseSpeed = 1000.0f;
	flowConfig.enemySpeedStep = 0.0f;
	flowConfig.enemyContactDamage = 0;
	flowConfig.playerSpawnSafeMargin = 0.0f;

	neon::Simulation simulation{ flowConfig };
	if (!runUntil(
		simulation,
		neon::GameState::UpgradeSelection,
		2)) {
		return 7;
	}

	const neon::GameSnapshot offerSnapshot =
		simulation.snapshot();
	if (offerSnapshot.upgradeOptionCount != 3 ||
		offerSnapshot.intermissionRemaining != 0.0f) {
		return 8;
	}

	neon::InputCommand pause{};
	pause.pausePressed = true;
	simulation.step(pause, fixedDt);
	if (simulation.state() != neon::GameState::Paused) {
		return 9;
	}
	simulation.step(neon::InputCommand{}, 1.0f);
	if (simulation.state() != neon::GameState::Paused) {
		return 10;
	}
	simulation.step(pause, fixedDt);
	if (simulation.state() !=
		neon::GameState::UpgradeSelection) {
		return 11;
	}

	constexpr int selectedOption = 1;
	const neon::UpgradeStats expectedStats =
		offerSnapshot.upgradeOptions[
			selectedOption
		].nextStats;
	neon::InputCommand selection{};
	selection.upgradeSelection = selectedOption;
	simulation.step(selection, fixedDt);

	const auto recordedInputs =
		simulation.consumeRecordedInputs();
	if (recordedInputs.empty() ||
		recordedInputs.back().command.upgradeSelection !=
			selectedOption) {
		return 12;
	}

	const neon::GameSnapshot selectedSnapshot =
		simulation.snapshot();
	if (selectedSnapshot.state !=
			neon::GameState::Intermission ||
		selectedSnapshot.upgradeOptionCount != 0 ||
		!sameStats(
			selectedSnapshot.upgradeStats,
			expectedStats) ||
		selectedSnapshot.player.maxHealth !=
			expectedStats.maximumHealth ||
		selectedSnapshot.player.ammoInMagazine !=
			expectedStats.magazineCapacity ||
		selectedSnapshot.player.magazineCapacity !=
			expectedStats.magazineCapacity ||
		selectedSnapshot.player.reloading ||
		!selectedSnapshot.projectiles.empty()) {
		return 12;
	}

	int offeredEvents = 0;
	int selectedEvents = 0;
	for (const neon::GameEvent& event :
		simulation.consumeEvents()) {
		if (event.type ==
			neon::GameEventType::UpgradeOffered) {
			++offeredEvents;
		}
		if (event.type ==
			neon::GameEventType::UpgradeSelected) {
			++selectedEvents;
		}
	}
	if (offeredEvents != 3 || selectedEvents != 1) {
		return 13;
	}

	simulation.reset();
	const neon::GameSnapshot resetSnapshot =
		simulation.snapshot();
	if (!sameStats(
		resetSnapshot.upgradeStats,
		neon::UpgradeSystem{}.stats(flowConfig)) ||
		resetSnapshot.player.maxHealth !=
			flowConfig.playerInitialHealth) {
		return 14;
	}

	return 0;
}
