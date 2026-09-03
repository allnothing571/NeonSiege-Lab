#include "core/Simulation.h"

int main() {
	neon::Simulation simulation{};

	simulation.consumeEvents();

	neon::InputCommand moveCommand{};
	moveCommand.movement = { 1.0f, 0.0f };
	moveCommand.aimPosition = { 600.0f, 270.0f };

	simulation.step(
		moveCommand,
		1.0f / 60.0f
	);

	neon::InputCommand pauseCommand{};
	pauseCommand.pausePressed = true;

	simulation.step(
		pauseCommand,
		1.0f / 60.0f
	);

	const auto frames =
		simulation.consumeRecordedInputs();

	const bool firstFramePassed =
		frames.size() == 2 &&
		frames[0].tick == 1 &&
		frames[0].command.movement.x == 1.0f &&
		frames[0].command.movement.y == 0.0f &&
		frames[0].command.pausePressed == false;

	const bool secondFramePassed =
		frames[1].tick == 2 &&
		frames[1].command.pausePressed == true;

	const bool queueDrained =
		simulation.consumeRecordedInputs().empty();

	return firstFramePassed &&
		secondFramePassed &&
		queueDrained
		? 0
		: 1;
}