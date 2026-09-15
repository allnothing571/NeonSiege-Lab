#include "core/Simulation.h"

namespace {

	constexpr float fixedDt = 1.0f / 60.0f;

	bool clearCurrentWave(neon::Simulation& simulation) {
		for (int stepIndex = 0;
			stepIndex < 1200;
			++stepIndex) {

			neon::InputCommand command{};
			const neon::GameSnapshot snapshot =
				simulation.snapshot();

			if (!snapshot.enemies.empty()) {
				const neon::Rect enemyBounds =
					snapshot.enemies.front().bounds;
				command.aimPosition = neon::Vec2{
					enemyBounds.x + enemyBounds.w / 2.0f,
					enemyBounds.y + enemyBounds.h / 2.0f
				};
				command.fireHeld = true;
			}

			simulation.step(command, fixedDt);
			if (simulation.state() == neon::GameState::Intermission) {
				return true;
			}
		}

		return false;
	}

	bool advanceToNextWave(neon::Simulation& simulation) {
		for (int stepIndex = 0;
			stepIndex < 180;
			++stepIndex) {

			simulation.step(neon::InputCommand{}, fixedDt);
			if (simulation.state() == neon::GameState::Playing &&
				simulation.snapshot().currentWave >= 2) {
				return true;
			}
		}

		return false;
	}

}

int main() {
	neon::GameplayConfig config{};
	config.worldBounds = neon::Rect{ 0.0f, 0.0f, 200.0f, 200.0f };
	config.playerStartPosition = neon::Vec2{ 76.0f, 76.0f };
	config.maximumWaves = 5;
	config.maximumEnemiesPerWave = 1;
	config.waveIntermissionDuration = 2.0f;
	config.enemyBaseSpeed = 0.0f;
	config.enemySpeedStep = 0.0f;
	config.enemyHealth = 1;
	config.enemyContactDamage = 0;
	config.playerProjectileDamage = 100;
	config.playerProjectileSpeed = 1200.0f;
	config.playerProjectileSpreadDegrees = 0.0f;
	config.magazineCapacity = 100;
	config.fireInterval = 0.0f;
	config.reloadDuration = 0.0f;
	config.playerSpawnSafeMargin = 0.0f;

	neon::Simulation simulation{ config };
	if (simulation.state() != neon::GameState::Playing ||
		simulation.snapshot().currentWave != 1) {
		return 1;
	}

	if (!clearCurrentWave(simulation) ||
		simulation.snapshot().currentWave != 1) {
		return 1;
	}

	const float pausedRemaining =
		simulation.snapshot().intermissionRemaining;
	neon::InputCommand pauseCommand{};
	pauseCommand.pausePressed = true;
	simulation.step(pauseCommand, fixedDt);
	if (simulation.state() != neon::GameState::Paused) {
		return 1;
	}

	for (int stepIndex = 0; stepIndex < 120; ++stepIndex) {
		simulation.step(neon::InputCommand{}, fixedDt);
	}
	if (simulation.state() != neon::GameState::Paused ||
		simulation.snapshot().intermissionRemaining != pausedRemaining) {
		return 1;
	}

	simulation.step(pauseCommand, fixedDt);
	if (simulation.state() != neon::GameState::Intermission ||
		!advanceToNextWave(simulation)) {
		return 1;
	}

	for (int stepIndex = 0;
		stepIndex < 10000 &&
		simulation.state() != neon::GameState::Victory;
		++stepIndex) {

		if (simulation.state() == neon::GameState::Intermission) {
			simulation.step(neon::InputCommand{}, fixedDt);
			continue;
		}

		const neon::GameSnapshot snapshot =
			simulation.snapshot();
		neon::InputCommand command{};
		if (!snapshot.enemies.empty()) {
			const neon::Rect bounds = snapshot.enemies.front().bounds;
			command.aimPosition = neon::Vec2{
				bounds.x + bounds.w / 2.0f,
				bounds.y + bounds.h / 2.0f
			};
			command.fireHeld = true;
		}
		simulation.step(command, fixedDt);
	}

	if (simulation.state() != neon::GameState::Victory ||
		simulation.snapshot().currentWave != config.maximumWaves) {
		return 1;
	}

	neon::InputCommand restartCommand{};
	restartCommand.restartPressed = true;
	simulation.step(restartCommand, fixedDt);
	if (simulation.state() != neon::GameState::Playing ||
		simulation.snapshot().currentWave != 1 ||
		simulation.snapshot().score != 0) {
		return 1;
	}

	neon::GameplayConfig gameOverConfig = config;
	gameOverConfig.maximumWaves = 1;
	gameOverConfig.maximumEnemiesPerWave = 1;
	gameOverConfig.worldBounds = neon::Rect{ 0.0f, 0.0f, 200.0f, 200.0f };
	gameOverConfig.playerStartPosition = neon::Vec2{ 76.0f, 76.0f };
	gameOverConfig.playerInitialHealth = 1;
	gameOverConfig.enemyContactDamage = 100;
	gameOverConfig.enemyBaseSpeed = 1000.0f;
	gameOverConfig.playerSpawnSafeMargin = 0.0f;

	neon::Simulation gameOverSimulation{ gameOverConfig };
	for (int stepIndex = 0;
		stepIndex < 600 &&
		gameOverSimulation.state() == neon::GameState::Playing;
		++stepIndex) {

		gameOverSimulation.step(neon::InputCommand{}, fixedDt);
	}

	if (gameOverSimulation.state() != neon::GameState::Gameover) {
		return 1;
	}

	gameOverSimulation.step(restartCommand, fixedDt);
	const neon::GameSnapshot restarted =
		gameOverSimulation.snapshot();
	if (restarted.state != neon::GameState::Playing ||
		restarted.player.health != gameOverConfig.playerInitialHealth ||
		restarted.currentWave != 1) {
		return 1;
	}

	return 0;
}
