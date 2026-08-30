#include "core/InputCommand.h"

int main() {
	neon::InputCommand command{};

	const bool defaultsPassed =
		command.movement.x == 0.0f &&
		command.movement.y == 0.0f &&
		command.aimPosition.x == 0.0f &&
		command.aimPosition.y == 0.0f &&
		!command.fireHeld &&
		!command.reloadPressed &&
		!command.pausePressed &&
		!command.restartPressed;

	command.movement = { 1.0f, -1.0f };
	command.aimPosition = { 320.0f, 180.0f };
	command.fireHeld = true;
	command.reloadPressed = true;
	command.pausePressed = true;
	command.restartPressed = true;

	const bool assignedValuesPassed =
		command.movement.x == 1.0f &&
		command.movement.y == -1.0f &&
		command.aimPosition.x == 320.0f &&
		command.aimPosition.y == 180.0f &&
		command.fireHeld &&
		command.reloadPressed &&
		command.pausePressed &&
		command.restartPressed;

	return defaultsPassed && assignedValuesPassed
		? 0
		: 1;
}