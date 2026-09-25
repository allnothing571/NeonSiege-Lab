#include <cmath>

#include "core/Collision.h"
#include "core/Obstacle.h"
#include "core/Simulation.h"

bool nearlyEqual(
	float first,
	float second) {

	constexpr float epsilon = 0.001f;

	return std::fabs(
		first - second
	) <= epsilon;
}

int main() {
	const neon::Obstacle wall{
		neon::Rect{
			100.0f,
			120.0f,
			200.0f,
			40.0f
	}
	};

	const neon::Rect wallBounds =
		wall.bounds();

	const neon::Rect overlapping{
		150.0f,
		130.0f,
		10.0f,
		10.0f
	};

	const neon::Rect separated{
		400.0f,
		130.0f,
		10.0f,
		10.0f
	};

	const neon::Rect edgeTouch{
		300.0f,
		130.0f,
		10.0f,
		10.0f
	};

	const neon::Simulation simulation{};
	const neon::GameSnapshot snapshot =
		simulation.snapshot();

	const bool simulationSnapshotPassed =
		snapshot.obstacles.size() == 5 &&
		snapshot.obstacles[0].bounds.x == 180.0f &&
		snapshot.obstacles[0].bounds.y == 120.0f &&
		snapshot.obstacles[0].bounds.w == 140.0f &&
		snapshot.obstacles[0].bounds.h == 28.0f &&
		snapshot.obstacles[1].bounds.x == 640.0f &&
		snapshot.obstacles[1].bounds.y == 120.0f &&
		snapshot.obstacles[1].bounds.w == 140.0f &&
		snapshot.obstacles[1].bounds.h == 28.0f &&
		snapshot.obstacles[2].bounds.x == 390.0f &&
		snapshot.obstacles[2].bounds.y == 360.0f &&
		snapshot.obstacles[2].bounds.w == 180.0f &&
		snapshot.obstacles[2].bounds.h == 30.0f;

	neon::GameplayConfig slideConfig{};
	slideConfig.playerStartPosition = {
		318.0f,
		148.0f
	};

	neon::Simulation slideSimulation{
		slideConfig
	};

	neon::InputCommand slideCommand{};
	slideCommand.movement = {
		0.0f,
		-1.0f
	};

	for (int stepIndex = 0;
		stepIndex < 10;
		++stepIndex) {

		slideSimulation.step(
			slideCommand,
			1.0f / 60.0f
		);
	}

	const neon::GameSnapshot slideSnapshot =
		slideSimulation.snapshot();
	const float movementInset =
		slideSnapshot.player.bounds.w * 0.125f;
	const neon::Rect slideMovementBounds{
		slideSnapshot.player.bounds.x + movementInset,
		slideSnapshot.player.bounds.y + movementInset,
		slideSnapshot.player.bounds.w - movementInset * 2.0f,
		slideSnapshot.player.bounds.h - movementInset * 2.0f
	};

	const bool slideMovementPassed =
		nearlyEqual(
			slideSnapshot.player.bounds.x,
			318.0f
		) &&
		nearlyEqual(
			slideSnapshot.player.bounds.y,
			108.0f
		) &&
		!neon::intersects(
			slideMovementBounds,
			slideSnapshot.obstacles[0].bounds
		);

	neon::GameplayConfig blockedConfig{};
	blockedConfig.playerStartPosition = {
		250.0f,
		148.0f
	};

	neon::Simulation blockedSimulation{
		blockedConfig
	};

	for (int stepIndex = 0;
		stepIndex < 10;
		++stepIndex) {

		blockedSimulation.step(
			slideCommand,
			1.0f / 60.0f
		);
	}

	const neon::GameSnapshot blockedSnapshot =
		blockedSimulation.snapshot();
	const float blockedMovementInset =
		blockedSnapshot.player.bounds.w * 0.125f;
	const neon::Rect blockedMovementBounds{
		blockedSnapshot.player.bounds.x + blockedMovementInset,
		blockedSnapshot.player.bounds.y + blockedMovementInset,
		blockedSnapshot.player.bounds.w -
			blockedMovementInset * 2.0f,
		blockedSnapshot.player.bounds.h -
			blockedMovementInset * 2.0f
	};

	const bool blockedMovementPassed =
		nearlyEqual(
			blockedSnapshot.player.bounds.x,
			250.0f
		) &&
		nearlyEqual(
			blockedSnapshot.player.bounds.y,
			144.0f
		) &&
		!neon::intersects(
			blockedMovementBounds,
			blockedSnapshot.obstacles[0].bounds
		);

	const bool passed =
		wallBounds.x == 100.0f &&
		wallBounds.y == 120.0f &&
		wallBounds.w == 200.0f &&
		wallBounds.h == 40.0f &&
		neon::intersects(wallBounds, overlapping) &&
		!neon::intersects(wallBounds, separated) &&
		!neon::intersects(wallBounds, edgeTouch) &&
		simulationSnapshotPassed &&
		slideMovementPassed &&
		blockedMovementPassed;

	return passed ? 0 : 1;
}
