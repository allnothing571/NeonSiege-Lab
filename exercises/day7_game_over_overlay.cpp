#include "core/Simulation.h"

int main() {
	neon::GameplayConfig config{};
	config.playerStartPosition = { 80.0f, 80.0f };
	config.playerInitialHealth = 1;
	config.enemyBaseSpeed = 140.0f;
	config.enemySpeedStep = 0.0f;

	neon::Simulation simulation{ config };
	neon::InputCommand idleCommand{};

	for (int stepIndex = 0;
		stepIndex < 600 &&
		simulation.state() == neon::GameState::Playing;
		++stepIndex) {

		simulation.step(
			idleCommand,
			1.0f / 60.0f
		);
	}

	const neon::GameSnapshot gameOverSnapshot =
		simulation.snapshot();

	const bool gameOverPassed =
		gameOverSnapshot.state == neon::GameState::Gameover &&
		gameOverSnapshot.player.health == 0 &&
		!gameOverSnapshot.player.alive;

	neon::InputCommand ordinaryCommand{};
	ordinaryCommand.movement = { 1.0f, 0.0f };

	simulation.step(
		ordinaryCommand,
		1.0f / 60.0f
	);

	const bool ordinaryInputBlocked =
		simulation.state() == neon::GameState::Gameover;

	neon::InputCommand restartCommand{};
	restartCommand.restartPressed = true;

	simulation.step(
		restartCommand,
		1.0f / 60.0f
	);

	const neon::GameSnapshot restartedSnapshot =
		simulation.snapshot();

	const bool restartPassed =
		restartedSnapshot.state == neon::GameState::Playing &&
		restartedSnapshot.player.alive &&
		restartedSnapshot.player.health ==
		config.playerInitialHealth &&
		restartedSnapshot.score == 0 &&
		restartedSnapshot.currentWave == 1 &&
		restartedSnapshot.projectiles.empty();

	return gameOverPassed &&
		ordinaryInputBlocked &&
		restartPassed
		? 0
		: 1;
}
