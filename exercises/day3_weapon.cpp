#include "core/GameplayConfig.h"
#include "core/Weapon.h"

int main() {
	const neon::GameplayConfig config{};

	neon::PlayerWeapon weapon{
		config.magazineCapacity,
		config.fireInterval,
		config.reloadDuration
	};

	const bool initialPassed =
		weapon.ammoInMagazine() == 12 &&
		weapon.magazineCapacity() == 12 &&
		!weapon.isReloading();

	weapon.update(
		1.0f / 60.0f,
		true,
		false
	);

	const bool firstShotPassed =
		weapon.fireRequested() &&
		weapon.consumeShot() &&
		weapon.ammoInMagazine() == 11 &&
		!weapon.consumeShot();

	weapon.update(
		1.0f / 60.0f,
		true,
		false
	);

	const bool cooldownPassed =
		!weapon.fireRequested() &&
		weapon.ammoInMagazine() == 11;

	weapon.update(
		config.fireInterval,
		true,
		false
	);

	const bool secondShotPassed =
		weapon.fireRequested() &&
		weapon.consumeShot() &&
		weapon.ammoInMagazine() == 10;

	weapon.update(
		config.fireInterval,
		true,
		false
	);
	weapon.consumeShot();

	weapon.update(
		config.fireInterval,
		true,
		false
	);

	const bool continuousFirePassed =
		weapon.fireRequested() &&
		weapon.consumeShot() &&
		weapon.ammoInMagazine() == 8;

	weapon.update(
		1.0f / 60.0f,
		false,
		true
	);

	const bool reloadStartedPassed =
		weapon.isReloading() &&
		!weapon.fireRequested();

	weapon.update(
		config.reloadDuration,
		true,
		false
	);

	const bool reloadFinishedPassed =
		!weapon.isReloading() &&
		weapon.ammoInMagazine() == 12;

	weapon.reset();

	const bool resetPassed =
		weapon.ammoInMagazine() == 12 &&
		!weapon.isReloading();

	return initialPassed &&
		firstShotPassed &&
		cooldownPassed &&
		secondShotPassed &&
		continuousFirePassed &&
		reloadStartedPassed &&
		reloadFinishedPassed &&
		resetPassed
		? 0
		: 1;
}