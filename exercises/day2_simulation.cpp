#include <algorithm>
#include <cmath>
#include <iostream>

#include "core/LineOfSight.h"
#include "core/Simulation.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

bool reportCheck(
	bool passed,
	const char* checkName)
{
	if (!passed) {
		std::cerr
			<< "Simulation check failed: "
			<< checkName
			<< '\n';
	}

	return passed;
}

float centerDistanceSquared(
	const neon::Rect& first,
	const neon::Rect& second) {

	const float firstCenterX =
		first.x + first.w / 2.0f;
	const float firstCenterY =
		first.y + first.h / 2.0f;

	const float secondCenterX =
		second.x + second.w / 2.0f;
	const float secondCenterY =
		second.y + second.h / 2.0f;

	const float deltaX =
		firstCenterX - secondCenterX;
	const float deltaY =
		firstCenterY - secondCenterY;

	return deltaX * deltaX +
		deltaY * deltaY;
}

int main() {
	neon::GameplayConfig simulationConfig{};
	simulationConfig.enemyBaseSpeed = 0.0f;
	simulationConfig.enemySpeedStep = 0.0f;
	simulationConfig.playerStartPosition = {
		100.0f,
		200.0f
	};

	const float initialPlayerX =
		simulationConfig.playerStartPosition.x;

	const float initialPlayerY =
		simulationConfig.playerStartPosition.y;

	const float playerCenterX =
		initialPlayerX +
		simulationConfig.playerSize / 2.0f;

	const float playerCenterY =
		initialPlayerY +
		simulationConfig.playerSize / 2.0f;

	const float halfSecondMovement =
		simulationConfig.playerSpeed * 0.5f;

	const float movedPlayerX =
		initialPlayerX + halfSecondMovement;

	const float resumedPlayerX =
		initialPlayerX +
		halfSecondMovement * 2.0f;

	neon::Simulation simulation{
		simulationConfig
	};

	neon::Simulation enemyMovementSimulation{};

	const neon::GameSnapshot beforeEnemyMovement =
		enemyMovementSimulation.snapshot();

	neon::InputCommand idleCommand{};

	enemyMovementSimulation.step(
		idleCommand,
		1.0f / 60.0f
	);

	neon::GameplayConfig lethalConfig{};
	lethalConfig.playerStartPosition = {
		80.0f,
		80.0f
	};
	lethalConfig.playerInitialHealth = 1;
	lethalConfig.enemyBaseSpeed = 140.0f;
	lethalConfig.enemySpeedStep = 0.0f;

	neon::Simulation lethalSimulation{
		lethalConfig
	};

	for (int stepIndex = 0;
		stepIndex < 600 &&
		lethalSimulation.state() ==
		neon::GameState::Playing;
		++stepIndex) {

		lethalSimulation.step(
			idleCommand,
			1.0f / 60.0f
		);
	}

	const neon::GameSnapshot gameOverSnapshot =
		lethalSimulation.snapshot();

	const bool contactDamagePassed =
		gameOverSnapshot.state ==
		neon::GameState::Gameover &&
		gameOverSnapshot.player.health == 0 &&
		!gameOverSnapshot.player.alive;

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
		centerDistanceSquared(
			afterEnemyMovement.enemies[0].bounds,
			afterEnemyMovement.player.bounds
		) <
		centerDistanceSquared(
			beforeEnemyMovement.enemies[0].bounds,
			beforeEnemyMovement.player.bounds
		);

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
		initialSnapshot.player.health == 100 &&
		nearlyEqual(initialBounds.x, initialPlayerX) &&
		nearlyEqual(initialBounds.y, initialPlayerY);

	neon::InputCommand moveCommand{};
	moveCommand.movement = { 1.0f, 0.0f };

	simulation.step(moveCommand, 0.5f);

	const neon::GameSnapshot movedSnapshot =
		simulation.snapshot();

	const bool movementPassed =
		nearlyEqual(movedSnapshot.player.bounds.x, movedPlayerX) &&
		nearlyEqual(movedSnapshot.player.bounds.y, initialPlayerY);

	neon::InputCommand pauseCommand{};
	pauseCommand.movement = { 1.0f, 0.0f };
	pauseCommand.pausePressed = true;

	simulation.step(pauseCommand, 0.5f);

	const neon::GameSnapshot pausedSnapshot =
		simulation.snapshot();

	const bool pauseEnteredPassed =
		pausedSnapshot.state == neon::GameState::Paused &&
		nearlyEqual(
			pausedSnapshot.player.bounds.x,
			movedPlayerX
		);

	simulation.step(moveCommand, 0.5f);

	const neon::GameSnapshot stillPausedSnapshot =
		simulation.snapshot();

	const bool pausedMovementBlocked =
		stillPausedSnapshot.state == neon::GameState::Paused &&
		nearlyEqual(stillPausedSnapshot.player.bounds.x, movedPlayerX);

	neon::InputCommand unpauseCommand{};
	unpauseCommand.pausePressed = true;

	simulation.step(unpauseCommand, 0.5f);

	const neon::GameSnapshot unpausedSnapshot =
		simulation.snapshot();

	const bool pauseExitedPassed =
		unpausedSnapshot.state == neon::GameState::Playing &&
		nearlyEqual(unpausedSnapshot.player.bounds.x, movedPlayerX);

	simulation.step(moveCommand, 0.0f);
	simulation.step(moveCommand, -1.0f);

	const neon::GameSnapshot invalidStepSnapshot =
		simulation.snapshot();

	const bool invalidStepBlocked =
		nearlyEqual(
			invalidStepSnapshot.player.bounds.x,
			movedPlayerX
		);

	simulation.step(moveCommand, 0.5f);

	const neon::GameSnapshot resumedSnapshot =
		simulation.snapshot();

	const bool resumedMovementPassed =
		nearlyEqual(resumedSnapshot.player.bounds.x, resumedPlayerX);

	simulation.reset();

	const neon::GameSnapshot resetSnapshot =
		simulation.snapshot();

	const bool resetDeterminismPassed =
		!resetSnapshot.enemies.empty() &&
		resetSnapshot.enemies.size() ==
			initialSnapshot.enemies.size() &&
		nearlyEqual(
			resetSnapshot.enemies[0].bounds.x,
			initialSnapshot.enemies[0].bounds.x
		) &&
		nearlyEqual(
			resetSnapshot.enemies[0].bounds.y,
			initialSnapshot.enemies[0].bounds.y
		);

	const bool snapshotPassed =
		resetSnapshot.state ==
			neon::GameState::Playing &&
		resetSnapshot.score == 0 &&
		resetSnapshot.currentWave == 1 &&
		resetSnapshot.enemies.size() == 3 &&
		resetSnapshot.projectiles.empty() &&
		resetSnapshot.player.health == 100 &&
		resetSnapshot.player.alive &&
		resetSnapshot.player.ammoInMagazine == 12 &&
		resetSnapshot.player.magazineCapacity == 12 &&
		!resetSnapshot.player.reloading &&
		nearlyEqual(
			resetSnapshot.player.bounds.x,
			initialPlayerX
		) &&
		nearlyEqual(
			resetSnapshot.player.bounds.y,
			initialPlayerY
		) &&
		nearlyEqual(
			resetSnapshot.aimPosition.x,
			playerCenterX
		) &&
		nearlyEqual(
			resetSnapshot.aimPosition.y,
			playerCenterY
		) &&
		resetDeterminismPassed;

	neon::GameplayConfig projectileConfig{};
	projectileConfig.maximumEnemiesPerWave = 0;
	projectileConfig.playerProjectileSpreadDegrees = 0.0f;

	const float projectileOriginX =
		projectileConfig.playerStartPosition.x +
		projectileConfig.playerSize / 2.0f;

	const float projectileOriginY =
		projectileConfig.playerStartPosition.y +
		projectileConfig.playerSize / 2.0f;

	const float projectileAimX =
		projectileOriginX + 320.0f;

	neon::Simulation projectileSimulation{
		projectileConfig
	};

	neon::InputCommand fireCommand{};
	fireCommand.aimPosition = {
		projectileAimX,
		projectileOriginY
	};
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
			projectileOriginX
		) &&
		nearlyEqual(
			firstShotSnapshot.projectiles[0].bounds.y,
			projectileOriginY
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
			projectileOriginX + 10.0f
		);

	for (int stepIndex = 0;
		stepIndex < 20 &&
		projectileSimulation.snapshot()
		.projectiles.size() < 2;
		++stepIndex) {

		projectileSimulation.step(
			fireCommand,
			1.0f / 60.0f
		);
	}

	const neon::GameSnapshot secondShotSnapshot =
		projectileSimulation.snapshot();

	const bool secondShotPassed =
		secondShotSnapshot.projectiles.size() == 2 &&
		secondShotSnapshot.player.ammoInMagazine == 10;

	neon::GameplayConfig spreadConfig{};
	spreadConfig.maximumEnemiesPerWave = 0;
	spreadConfig.randomSeed = 4242u;
	spreadConfig.playerProjectileSpreadDegrees = 3.0f;

	neon::Simulation firstSpreadSimulation{
		spreadConfig
	};

	neon::Simulation secondSpreadSimulation{
		spreadConfig
	};

	neon::InputCommand spreadFireCommand{};
	spreadFireCommand.aimPosition = {
		projectileAimX,
		projectileOriginY
	};
	spreadFireCommand.fireHeld = true;

	firstSpreadSimulation.step(
		spreadFireCommand,
		1.0f / 60.0f
	);

	secondSpreadSimulation.step(
		spreadFireCommand,
		1.0f / 60.0f
	);

	neon::InputCommand spreadIdleCommand{};
	spreadIdleCommand.aimPosition = {
		projectileAimX,
		projectileOriginY
	};

	firstSpreadSimulation.step(
		spreadIdleCommand,
		1.0f / 60.0f
	);

	secondSpreadSimulation.step(
		spreadIdleCommand,
		1.0f / 60.0f
	);

	const neon::GameSnapshot firstSpreadSnapshot =
		firstSpreadSimulation.snapshot();

	const neon::GameSnapshot secondSpreadSnapshot =
		secondSpreadSimulation.snapshot();

	const bool deterministicSpreadPassed =
		firstSpreadSnapshot.projectiles.size() == 1 &&
		secondSpreadSnapshot.projectiles.size() == 1 &&
		nearlyEqual(
			firstSpreadSnapshot.projectiles[0].bounds.x,
			secondSpreadSnapshot.projectiles[0].bounds.x
		) &&
		nearlyEqual(
			firstSpreadSnapshot.projectiles[0].bounds.y,
			secondSpreadSnapshot.projectiles[0].bounds.y
		);

	const float spreadDeltaX =
		firstSpreadSnapshot.projectiles[0].bounds.x -
		projectileOriginX;

	const float spreadDeltaY =
		firstSpreadSnapshot.projectiles[0].bounds.y -
		projectileOriginY;

	constexpr float maximumSpreadRadians =
		3.14159265358979323846f / 60.0f;

	const bool spreadRangePassed =
		std::fabs(
			std::atan2(
				spreadDeltaY,
				spreadDeltaX
			)
		) <= maximumSpreadRadians + 0.001f;

	neon::Simulation zeroAimSimulation{
	projectileConfig
	};

	neon::InputCommand zeroAimFire{};
	zeroAimFire.aimPosition = {
		projectileOriginX,
		projectileOriginY
	};
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
	hitConfig.enemyBaseSpeed = 0.0f;
	hitConfig.enemySpeedStep = 0.0f;

	neon::Simulation hitSimulation{
		hitConfig
	};

	const neon::GameSnapshot beforeHit =
		hitSimulation.snapshot();

	const neon::Vec2 hitPlayerCenter{
		beforeHit.player.bounds.x +
			beforeHit.player.bounds.w / 2.0f,
		beforeHit.player.bounds.y +
			beforeHit.player.bounds.h / 2.0f
	};

	const auto visibleEnemy =
		std::find_if(
			beforeHit.enemies.begin(),
			beforeHit.enemies.end(),
			[&beforeHit, hitPlayerCenter](
				const neon::EnemySnapshot& enemy) {

				const neon::Vec2 enemyCenter{
					enemy.bounds.x +
						enemy.bounds.w / 2.0f,
					enemy.bounds.y +
						enemy.bounds.h / 2.0f
				};

				return std::none_of(
					beforeHit.obstacles.begin(),
					beforeHit.obstacles.end(),
					[hitPlayerCenter, enemyCenter](
						const neon::ObstacleSnapshot&
							obstacle) {

						return neon::segmentIntersectsRect(
							hitPlayerCenter,
							enemyCenter,
							obstacle.bounds
						);
					}
				);
			}
		);

	const bool visibleTargetFound =
		visibleEnemy != beforeHit.enemies.end();

	const neon::Rect targetBounds =
		visibleTargetFound
		? visibleEnemy->bounds
		: neon::Rect{};

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
		stepIndex < 180 &&
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
		visibleTargetFound &&
		afterHit.score == 175 &&
		afterHit.currentWave == 1 &&
		afterHit.enemies.size() == 2 &&
		afterHit.projectiles.empty();

	const bool passed =
		reportCheck(
			initialStatePassed,
			"initialStatePassed"
		) &
		reportCheck(
			movementPassed,
			"movementPassed"
		) &
		reportCheck(
			pauseEnteredPassed,
			"pauseEnteredPassed"
		) &
		reportCheck(
			pausedMovementBlocked,
			"pausedMovementBlocked"
		) &
		reportCheck(
			pauseExitedPassed,
			"pauseExitedPassed"
		) &
		reportCheck(
			invalidStepBlocked,
			"invalidStepBlocked"
		) &
		reportCheck(
			resumedMovementPassed,
			"resumedMovementPassed"
		) &
		reportCheck(
			snapshotPassed,
			"snapshotPassed"
		) &
		reportCheck(
			enemyMovementPassed,
			"enemyMovementPassed"
		) &
		reportCheck(
			contactDamagePassed,
			"contactDamagePassed"
		) &
		reportCheck(
			restartPassed,
			"restartPassed"
		) &
		reportCheck(
			firstShotPassed,
			"firstShotPassed"
		) &
		reportCheck(
			heldFirePassed,
			"heldFirePassed"
		) &
		reportCheck(
			secondShotPassed,
			"secondShotPassed"
		) &
		reportCheck(
			deterministicSpreadPassed,
			"deterministicSpreadPassed"
		) &
		reportCheck(
			spreadRangePassed,
			"spreadRangePassed"
		) &
		reportCheck(
			zeroAimBlocked,
			"zeroAimBlocked"
		) &
		reportCheck(
			projectileHitPassed,
			"projectileHitPassed"
		);

	return passed ? 0 : 1;
}
