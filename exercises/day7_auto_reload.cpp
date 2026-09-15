#include <algorithm>

#include "core/Simulation.h"
#include "core/Weapon.h"

namespace {

bool containsReloadEvent(
	const std::vector<neon::GameEvent>& events,
	neon::GameEventType type,
	int value) {
	return std::any_of(
		events.begin(),
		events.end(),
		[type, value](const neon::GameEvent& event) {
			return event.type == type &&
				event.value == value &&
				event.sourceKind ==
				neon::GameEntityKind::Player;
		}
	);
}

bool weaponStartsReloadAfterLastShot() {
	neon::PlayerWeapon weapon{
		2,
		0.0f,
		0.05f
	};

	weapon.update(
		1.0f / 60.0f,
		true,
		false
	);

	if (!weapon.fireRequested() ||
		!weapon.consumeShot() ||
		weapon.ammoInMagazine() != 1 ||
		weapon.isReloading()) {

		return false;
	}

	weapon.update(
		1.0f / 60.0f,
		true,
		false
	);

	if (!weapon.fireRequested() ||
		!weapon.consumeShot() ||
		weapon.ammoInMagazine() != 0 ||
		!weapon.isReloading() ||
		!weapon.reloadStartedThisUpdate()) {

		return false;
	}

	weapon.update(
		0.05f,
		false,
		false
	);

	return !weapon.isReloading() &&
		weapon.ammoInMagazine() == 2 &&
		weapon.reloadCompletedThisUpdate();
}

bool simulationRecordsAutomaticReload() {
	neon::GameplayConfig config{};
	config.maximumEnemiesPerWave = 0;
	config.magazineCapacity = 1;
	config.fireInterval = 0.0f;
	config.reloadDuration = 0.05f;
	config.playerProjectileSpreadDegrees = 0.0f;

	neon::Simulation simulation(config);
	simulation.consumeEvents();

	neon::InputCommand fireCommand{};
	fireCommand.aimPosition = {
		0.0f,
		0.0f
	};
	fireCommand.fireHeld = true;

	simulation.step(
		fireCommand,
		1.0f / 60.0f
	);

	const auto shotEvents =
		simulation.consumeEvents();
	const neon::GameSnapshot afterShot =
		simulation.snapshot();

	const bool shotAndReloadStarted =
		std::any_of(
			shotEvents.begin(),
			shotEvents.end(),
			[](const neon::GameEvent& event) {
				return event.type ==
					neon::GameEventType::PlayerShot;
			}
		) &&
		containsReloadEvent(
			shotEvents,
			neon::GameEventType::ReloadStarted,
			0
		) &&
		afterShot.player.ammoInMagazine == 0 &&
		afterShot.player.reloading;

	if (!shotAndReloadStarted) {
		return false;
	}

	std::vector<neon::GameEvent> completedEvents;
	for (int frame = 0;
		frame < 10 &&
		simulation.snapshot().player.reloading;
		++frame) {

		neon::InputCommand idleCommand{};
		simulation.step(
			idleCommand,
			1.0f / 60.0f
		);

		const auto frameEvents =
			simulation.consumeEvents();
		completedEvents.insert(
			completedEvents.end(),
			frameEvents.begin(),
			frameEvents.end()
		);
	}

	const neon::GameSnapshot afterReload =
		simulation.snapshot();

	return containsReloadEvent(
		completedEvents,
		neon::GameEventType::ReloadCompleted,
		1
	) &&
		afterReload.player.ammoInMagazine == 1 &&
		!afterReload.player.reloading;
}

}

int main() {
	return weaponStartsReloadAfterLastShot() &&
		simulationRecordsAutomaticReload()
		? 0
		: 1;
}
