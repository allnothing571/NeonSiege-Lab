#include "core/Simulation.h"

namespace {

	constexpr float fixedDt = 1.0f / 60.0f;

	bool contains(
		const std::vector<neon::PresentationEvent>& events,
		neon::PresentationEventType type) {

		for (const neon::PresentationEvent& event : events) {
			if (event.type == type) {
				return true;
			}
		}
		return false;
	}

}

int main() {
	neon::GameplayConfig config{};
	config.worldBounds = neon::Rect{
		0.0f,
		0.0f,
		960.0f,
		540.0f
	};
	config.playerStartPosition = neon::Vec2{
		456.0f,
		246.0f
	};
	config.maximumWaves = 1;
	config.maximumEnemiesPerWave = 1;
	config.enemyBaseSpeed = 0.0f;
	config.enemySpeedStep = 0.0f;
	config.enemyHealth = 10;
	config.enemyContactDamage = 0;
	config.playerProjectileDamage = 10;
	config.playerProjectileSpeed = 1200.0f;
	config.playerProjectileSpreadDegrees = 0.0f;
	config.magazineCapacity = 10;
	config.fireInterval = 0.0f;
	config.reloadDuration = 0.0f;

	neon::Simulation simulation{ config };
	simulation.consumeEvents();
	simulation.consumePresentationEvents();

	bool sawTelemetryShot = false;
	bool sawProjectileHit = false;
	bool sawEnemyDamaged = false;
	bool sawEnemyDied = false;

	for (int stepIndex = 0;
		stepIndex < 600 &&
		!sawEnemyDied;
		++stepIndex) {

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

		const std::vector<neon::PresentationEvent> presentationEvents =
			simulation.consumePresentationEvents();
		const std::vector<neon::GameEvent> telemetryEvents =
			simulation.consumeEvents();

		for (const neon::GameEvent& event : telemetryEvents) {
			if (event.type == neon::GameEventType::PlayerShot) {
				sawTelemetryShot = true;
			}
		}

		sawProjectileHit =
			sawProjectileHit ||
			contains(
				presentationEvents,
				neon::PresentationEventType::ProjectileHit
			);
		sawEnemyDamaged =
			sawEnemyDamaged ||
			contains(
				presentationEvents,
				neon::PresentationEventType::EnemyDamaged
			);
		sawEnemyDied =
			sawEnemyDied ||
			contains(
				presentationEvents,
				neon::PresentationEventType::EnemyDied
			);
	}

	if (!sawTelemetryShot ||
		!sawProjectileHit ||
		!sawEnemyDamaged ||
		!sawEnemyDied) {
		return 1;
	}

	simulation.reset();
	if (!simulation.consumePresentationEvents().empty()) {
		return 1;
	}

	neon::GameplayConfig damageConfig = config;
	damageConfig.worldBounds = neon::Rect{ 0.0f, 0.0f, 200.0f, 200.0f };
	damageConfig.playerStartPosition = neon::Vec2{ 76.0f, 76.0f };
	damageConfig.playerInitialHealth = 100;
	damageConfig.enemyContactDamage = 10;
	damageConfig.enemyBaseSpeed = 1000.0f;
	damageConfig.playerSpawnSafeMargin = 0.0f;

	neon::Simulation damageSimulation{ damageConfig };
	bool sawPlayerDamaged = false;
	for (int stepIndex = 0;
		stepIndex < 600 &&
		!sawPlayerDamaged;
		++stepIndex) {

		damageSimulation.step(neon::InputCommand{}, fixedDt);
		const auto events =
			damageSimulation.consumePresentationEvents();
		sawPlayerDamaged = contains(
			events,
			neon::PresentationEventType::PlayerDamaged
		);
	}

	return sawPlayerDamaged ? 0 : 1;
}
