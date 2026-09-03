#include "core/Simulation.h"

int main() {
	neon::Simulation simulation{};

	const auto initialEvents =
		simulation.consumeEvents();

	const bool waveStarted =
		initialEvents.size() == 1 &&
		initialEvents[0].type ==
		neon::GameEventType::WaveStarted &&
		initialEvents[0].tick == 0 &&
		initialEvents[0].value == 1;

	neon::InputCommand fireCommand{};
	fireCommand.aimPosition = { 0.0f, 0.0f };
	fireCommand.fireHeld = true;

	simulation.step(
		fireCommand,
		1.0f / 60.0f
	);

	const auto shotEvents =
		simulation.consumeEvents();

	const bool playerShot =
		simulation.tick() == 1 &&
		!shotEvents.empty() &&
		shotEvents[0].type ==
		neon::GameEventType::PlayerShot &&
		shotEvents[0].tick == 1 &&
		shotEvents[0].sourceKind ==
		neon::GameEntityKind::Player &&
		shotEvents[0].sourceId ==
		neon::playerEntityId;

	const bool queueDrained =
		simulation.consumeEvents().empty();

	neon::InputCommand pauseCommand{};
	pauseCommand.pausePressed = true;

	simulation.step(
		pauseCommand,
		1.0f / 60.0f
	);

	const auto pauseEvents =
		simulation.consumeEvents();

	const bool pauseRecorded =
		simulation.state() ==
		neon::GameState::Paused &&
		pauseEvents.size() == 1 &&
		pauseEvents[0].type ==
		neon::GameEventType::GameStateChanged &&
		pauseEvents[0].value ==
		static_cast<int>(
			neon::GameState::Paused
			);

	return waveStarted &&
		playerShot &&
		queueDrained &&
		pauseRecorded
		? 0
		: 1;
}