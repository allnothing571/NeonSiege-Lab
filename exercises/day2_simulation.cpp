#include <cmath>

#include "core/Simulation.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

int main() {
	neon::Simulation simulation{};

	neon::Simulation enemyMovementSimulation{};

	const neon::GameSnapshot beforeEnemyMovement =
		enemyMovementSimulation.snapshot();

	neon::InputCommand idleCommand{};

	enemyMovementSimulation.step(
		idleCommand,
		1.0f / 60.0f
	);

	neon::GameplayConfig lethalConfig{};
	lethalConfig.playerStartPosition = { 80.0f, 80.0f };
	lethalConfig.playerInitialHealth = 1;

	neon::Simulation lethalSimulation{
		lethalConfig
	};

	lethalSimulation.step(
		idleCommand,
		0.001f
	);

	const neon::GameSnapshot gameOverSnapshot =
		lethalSimulation.snapshot();

	const bool contactDamagePassed =
		gameOverSnapshot.state ==
		neon::GameState::Gameover &&
		gameOverSnapshot.player.health == 0 &&
		!gameOverSnapshot.player.alive &&
		gameOverSnapshot.enemies.size() == 2;

	neon::InputCommand restartCommand{};
	restartCommand.restartPressed = true;

	lethalSimulation.step(
		restartCommand,
		1.0f / 60.0f
	);

	const neon::GameSnapshot restartedSnapshot =
		lethalSimulation.snapshot();

	const bool restartPassed =
		restartedSnapshot.state ==
		neon::GameState::Playing &&
		restartedSnapshot.player.health == 1 &&
		restartedSnapshot.player.alive &&
		restartedSnapshot.enemies.size() == 3 &&
		restartedSnapshot.currentWave == 1 &&
		restartedSnapshot.score == 0;

	const neon::GameSnapshot afterEnemyMovement =
		enemyMovementSimulation.snapshot();

	const bool enemyMovementPassed =
		afterEnemyMovement.enemies[0].bounds.x >
		beforeEnemyMovement.enemies[0].bounds.x &&
		afterEnemyMovement.enemies[0].bounds.y >
		beforeEnemyMovement.enemies[0].bounds.y;

	const neon::GameSnapshot initialSnapshot =
		simulation.snapshot();

	const neon::Rect initialBounds =
		initialSnapshot.player.bounds;

	const bool initialStatePassed =
		initialSnapshot.state == neon::GameState::Playing &&
		initialSnapshot.score == 0 &&
		initialSnapshot.currentWave == 1 &&
		initialSnapshot.enemies.size() == 3 &&
		initialSnapshot.projectiles.empty() &&
		initialSnapshot.player.health == 3 &&
		nearlyEqual(initialBounds.x, 456.0f) &&
		nearlyEqual(initialBounds.y, 246.0f);

	neon::InputCommand moveCommand{};
	moveCommand.movement = { 1.0f, 0.0f };

	simulation.step(moveCommand, 0.5f);

	const neon::GameSnapshot movedSnapshot =
		simulation.snapshot();

	const bool movementPassed =
		nearlyEqual(movedSnapshot.player.bounds.x, 576.0f) &&
		nearlyEqual(movedSnapshot.player.bounds.y, 246.0f);

	neon::InputCommand pauseCommand{};
	pauseCommand.movement = { 1.0f, 0.0f };
	pauseCommand.pausePressed = true;

	simulation.step(pauseCommand, 0.5f);

	const neon::GameSnapshot pausedSnapshot =
		simulation.snapshot();

	const bool pauseEnteredPassed =
		pausedSnapshot.state == neon::GameState::Paused &&
		nearlyEqual(pausedSnapshot.player.bounds.x, 576.0f);

	simulation.step(moveCommand, 0.5f);

	const neon::GameSnapshot stillPausedSnapshot =
		simulation.snapshot();

	const bool pausedMovementBlocked =
		stillPausedSnapshot.state == neon::GameState::Paused &&
		nearlyEqual(stillPausedSnapshot.player.bounds.x, 576.0f);

	neon::InputCommand unpauseCommand{};
	unpauseCommand.pausePressed = true;

	simulation.step(unpauseCommand, 0.5f);

	const neon::GameSnapshot unpausedSnapshot =
		simulation.snapshot();

	const bool pauseExitedPassed =
		unpausedSnapshot.state == neon::GameState::Playing &&
		nearlyEqual(unpausedSnapshot.player.bounds.x, 576.0f);

	simulation.step(moveCommand, 0.0f);
	simulation.step(moveCommand, -1.0f);

	const neon::GameSnapshot invalidStepSnapshot =
		simulation.snapshot();

	const bool invalidStepBlocked =
		nearlyEqual(invalidStepSnapshot.player.bounds.x, 576.0f);

	simulation.step(moveCommand, 0.5f);

	const neon::GameSnapshot resumedSnapshot =
		simulation.snapshot();

	const bool resumedMovementPassed =
		nearlyEqual(resumedSnapshot.player.bounds.x, 696.0f);

	simulation.reset();

	const neon::GameSnapshot resetSnapshot =
		simulation.snapshot();

	const bool snapshotPassed =
		resetSnapshot.state == neon::GameState::Playing &&
		resetSnapshot.score == 0 &&
		resetSnapshot.currentWave == 1 &&
		resetSnapshot.enemies.size() == 3 &&
		resetSnapshot.projectiles.empty() &&
		resetSnapshot.player.health == 3 &&
		resetSnapshot.player.alive &&
		nearlyEqual(resetSnapshot.player.bounds.x, 456.0f) &&
		nearlyEqual(resetSnapshot.player.bounds.y, 246.0f) &&
		nearlyEqual(resetSnapshot.aimPosition.x, 480.0f) &&
		nearlyEqual(resetSnapshot.aimPosition.y, 270.0f);

	neon::Simulation projectileSimulation{};

	neon::InputCommand fireCommand{};
	fireCommand.aimPosition = { 800.0f, 270.0f };
	fireCommand.fireHeld = true;

	projectileSimulation.step(
		fireCommand,
		1.0f / 60.0f
	);

	const neon::GameSnapshot firstShotSnapshot =
		projectileSimulation.snapshot();

	const bool firstShotPassed =
		firstShotSnapshot.projectiles.size() == 1 &&
		nearlyEqual(
			firstShotSnapshot.projectiles[0].bounds.x,
			480.0f
		) &&
		nearlyEqual(
			firstShotSnapshot.projectiles[0].bounds.y,
			270.0f
		);

	projectileSimulation.step(
		fireCommand,
		1.0f / 60.0f
	);

	const neon::GameSnapshot heldSnapshot =
		projectileSimulation.snapshot();

	const bool heldFirePassed =
		heldSnapshot.projectiles.size() == 1 &&
		nearlyEqual(
			heldSnapshot.projectiles[0].bounds.x,
			490.0f
		);

	neon::InputCommand releaseCommand{};
	releaseCommand.aimPosition = { 800.0f, 270.0f };

	projectileSimulation.step(
		releaseCommand,
		1.0f / 60.0f
	);

	projectileSimulation.step(
		fireCommand,
		1.0f / 60.0f
	);

	const bool secondPressPassed =
		projectileSimulation.snapshot()
		.projectiles.size() == 2;

	neon::Simulation zeroAimSimulation{};

	neon::InputCommand zeroAimFire{};
	zeroAimFire.aimPosition = { 480.0f, 270.0f };
	zeroAimFire.fireHeld = true;

	zeroAimSimulation.step(
		zeroAimFire,
		1.0f / 60.0f
	);

	const bool zeroAimBlocked =
		zeroAimSimulation.snapshot()
		.projectiles.empty();

	neon::GameplayConfig hitConfig{};
	hitConfig.playerProjectileDamage = 30;
	hitConfig.scorePerEnemy = 175;

	neon::Simulation hitSimulation{
		hitConfig
	};

	const neon::GameSnapshot beforeHit =
		hitSimulation.snapshot();

	const neon::Rect targetBounds =
		beforeHit.enemies[0].bounds;

	const neon::Vec2 targetCenter{
		targetBounds.x + targetBounds.w / 2.0f,
		targetBounds.y + targetBounds.h / 2.0f
	};

	neon::InputCommand hitFireCommand{};
	hitFireCommand.aimPosition = targetCenter;
	hitFireCommand.fireHeld = true;

	hitSimulation.step(
		hitFireCommand,
		1.0f / 60.0f
	);

	neon::InputCommand hitReleaseCommand{};
	hitReleaseCommand.aimPosition = targetCenter;

	for (int stepIndex = 0;
		stepIndex < 90 &&
		hitSimulation.snapshot().score == 0;
		++stepIndex) {

		hitSimulation.step(
			hitReleaseCommand,
			1.0f / 60.0f
		);
	}

	const neon::GameSnapshot afterHit =
		hitSimulation.snapshot();

	const bool projectileHitPassed =
		afterHit.score == 175 &&
		afterHit.currentWave == 1 &&
		afterHit.enemies.size() == 2 &&
		afterHit.projectiles.empty();

	const bool passed =
		initialStatePassed &&
		movementPassed &&
		pauseEnteredPassed &&
		pausedMovementBlocked &&
		pauseExitedPassed &&
		invalidStepBlocked &&
		resumedMovementPassed &&
		snapshotPassed &&
		enemyMovementPassed &&
		contactDamagePassed &&
		restartPassed &&
		firstShotPassed &&
		heldFirePassed &&
		secondPressPassed &&
		zeroAimBlocked &&
		projectileHitPassed;

	return passed ? 0 : 1;
}
