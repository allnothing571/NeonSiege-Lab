#include "core/Simulation.h"

#include <iostream>

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

	int count(
		const std::vector<neon::PresentationEvent>& events,
		neon::PresentationEventType type) {

		int result = 0;
		for (const neon::PresentationEvent& event : events) {
			if (event.type == type) {
				++result;
			}
		}
		return result;
	}

	bool contains(
		const std::vector<neon::GameEvent>& events,
		neon::GameEventType type) {

		for (const neon::GameEvent& event : events) {
			if (event.type == type) {
				return true;
			}
		}
		return false;
	}

	bool reloadPresentationEventsAreSeparated() {
		neon::GameplayConfig config{};
		config.magazineCapacity = 2;
		config.fireInterval = 0.0f;
		config.reloadDuration = 0.10f;
		config.enemyBaseSpeed = 0.0f;
		config.enemyContactDamage = 0;

		neon::Simulation manualReload{ config };
		manualReload.consumeEvents();
		manualReload.consumePresentationEvents();

		neon::InputCommand fire{};
		fire.fireHeld = true;
		fire.aimPosition = neon::Vec2{ 900.0f, 270.0f };
		manualReload.step(fire, fixedDt);
		manualReload.consumeEvents();
		manualReload.consumePresentationEvents();

		neon::InputCommand reload{};
		reload.reloadPressed = true;
		manualReload.step(reload, fixedDt);
		const auto started =
			manualReload.consumePresentationEvents();
		manualReload.consumeEvents();
		if (count(started, neon::PresentationEventType::ReloadStarted) != 1 ||
			count(started, neon::PresentationEventType::ReloadCompleted) != 0) {

			std::cerr << "manual reload did not emit start-only event\n";
			return false;
		}

		manualReload.step(reload, fixedDt);
		const auto repeated =
			manualReload.consumePresentationEvents();
		manualReload.consumeEvents();
		if (contains(repeated, neon::PresentationEventType::ReloadStarted) ||
			contains(repeated, neon::PresentationEventType::ReloadCompleted)) {

			std::cerr << "repeat reload input emitted another event\n";
			return false;
		}

		int startsWhileReloading = 0;
		int completions = 0;
		for (int index = 0;
			index < 20 && manualReload.snapshot().player.reloading;
			++index) {

			manualReload.step(neon::InputCommand{}, fixedDt);
			const auto events =
				manualReload.consumePresentationEvents();
			manualReload.consumeEvents();
			startsWhileReloading += count(
				events,
				neon::PresentationEventType::ReloadStarted
			);
			completions += count(
				events,
				neon::PresentationEventType::ReloadCompleted
			);
		}
		if (manualReload.snapshot().player.reloading ||
			startsWhileReloading != 0 || completions != 1) {

			std::cerr << "manual reload completion event count is wrong\n";
			return false;
		}

		neon::Simulation fullMagazine{ config };
		fullMagazine.consumeEvents();
		fullMagazine.consumePresentationEvents();
		fullMagazine.step(reload, fixedDt);
		const auto fullMagazineEvents =
			fullMagazine.consumePresentationEvents();
		if (contains(
				fullMagazineEvents,
				neon::PresentationEventType::ReloadStarted) ||
			contains(
				fullMagazineEvents,
				neon::PresentationEventType::ReloadCompleted)) {

			std::cerr << "full magazine emitted a reload event\n";
			return false;
		}

		neon::GameplayConfig automaticConfig = config;
		automaticConfig.magazineCapacity = 1;
		neon::Simulation automaticReload{ automaticConfig };
		automaticReload.consumeEvents();
		automaticReload.consumePresentationEvents();
		automaticReload.step(fire, fixedDt);
		const auto automaticEvents =
			automaticReload.consumePresentationEvents();
		if (count(
				automaticEvents,
				neon::PresentationEventType::ReloadStarted) != 1 ||
			contains(
				automaticEvents,
				neon::PresentationEventType::ReloadCompleted)) {

			std::cerr << "automatic reload start event is wrong\n";
			return false;
		}

		neon::GameplayConfig instantConfig = automaticConfig;
		instantConfig.reloadDuration = 0.0f;
		neon::Simulation instantReload{ instantConfig };
		instantReload.consumeEvents();
		instantReload.consumePresentationEvents();
		instantReload.step(fire, fixedDt);
		const auto instantPresentation =
			instantReload.consumePresentationEvents();
		const auto instantTelemetry = instantReload.consumeEvents();
		if (contains(
				instantPresentation,
				neon::PresentationEventType::ReloadStarted) ||
			count(
				instantPresentation,
				neon::PresentationEventType::ReloadCompleted) != 1 ||
			!contains(
				instantTelemetry,
				neon::GameEventType::ReloadStarted) ||
			!contains(
				instantTelemetry,
				neon::GameEventType::ReloadCompleted)) {

			std::cerr << "instant reload presentation or telemetry is wrong\n";
			return false;
		}

		return true;
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
	const auto initialPresentationEvents =
		simulation.consumePresentationEvents();
	if (!contains(
		initialPresentationEvents,
		neon::PresentationEventType::WaveStarted)) {

		return 1;
	}

	bool sawTelemetryShot = false;
	bool sawPresentationShot = false;
	bool sawProjectileHit = false;
	bool sawEnemyDamaged = false;
	bool sawEnemyDied = false;
	bool sawVictory = false;

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
		sawPresentationShot =
			sawPresentationShot ||
			contains(
				presentationEvents,
				neon::PresentationEventType::PlayerShot
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
		sawVictory =
			sawVictory ||
			contains(
				presentationEvents,
				neon::PresentationEventType::Victory
			);
	}

	if (!sawTelemetryShot ||
		!sawPresentationShot ||
		!sawProjectileHit ||
		!sawEnemyDamaged ||
		!sawEnemyDied ||
		!sawVictory) {

		std::cerr
			<< "shot telemetry=" << sawTelemetryShot
			<< " presentation=" << sawPresentationShot
			<< " hit=" << sawProjectileHit
			<< " damaged=" << sawEnemyDamaged
			<< " died=" << sawEnemyDied
			<< " victory=" << sawVictory
			<< '\n';
		return 1;
	}

	simulation.reset();
	const auto resetEvents =
		simulation.consumePresentationEvents();
	if (!contains(
		resetEvents,
		neon::PresentationEventType::WaveStarted) ||
		!simulation.consumePresentationEvents().empty()) {

		return 1;
	}

	neon::GameplayConfig damageConfig = config;
	damageConfig.worldBounds = neon::Rect{ 0.0f, 0.0f, 200.0f, 200.0f };
	damageConfig.playerStartPosition = neon::Vec2{ 76.0f, 76.0f };
	damageConfig.playerInitialHealth = 100;
	damageConfig.enemyContactDamage = 100;
	damageConfig.enemyBaseSpeed = 1000.0f;
	damageConfig.playerSpawnSafeMargin = 0.0f;

	neon::Simulation damageSimulation{ damageConfig };
	bool sawPlayerDamaged = false;
	bool sawGameOver = false;
	for (int stepIndex = 0;
		stepIndex < 600 &&
		!sawGameOver;
		++stepIndex) {

		damageSimulation.step(neon::InputCommand{}, fixedDt);
		const auto events =
			damageSimulation.consumePresentationEvents();
		sawPlayerDamaged = sawPlayerDamaged || contains(
			events,
			neon::PresentationEventType::PlayerDamaged
		);
		sawGameOver = sawGameOver || contains(
			events,
			neon::PresentationEventType::GameOver
		);
	}

	if (!sawPlayerDamaged || !sawGameOver) {
		std::cerr
			<< "player damaged=" << sawPlayerDamaged
			<< " game over=" << sawGameOver
			<< '\n';
		return 1;
	}

	if (!reloadPresentationEventsAreSeparated()) {
		return 1;
	}

	return 0;
}
