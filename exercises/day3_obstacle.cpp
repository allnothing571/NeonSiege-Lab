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
		snapshot.obstacles.size() == 2 &&
		snapshot.obstacles[0].bounds.w == 240.0f &&
		snapshot.obstacles[0].bounds.h == 32.0f &&
		snapshot.obstacles[1].bounds.w == 180.0f &&
		snapshot.obstacles[1].bounds.h == 40.0f &&
		snapshot.obstacles[1].bounds.x == 620.0f &&
		snapshot.obstacles[1].bounds.y == 320.0f;

	neon::GameplayConfig slideConfig{};
	slideConfig.playerStartPosition = {
		260.0f,
		212.0f
	};

	neon::Simulation slideSimulation{
		slideConfig
	};

	neon::InputCommand slideCommand{};
	slideCommand.movement = {
		1.0f,
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

	const bool slideMovementPassed =
		nearlyEqual(
			slideSnapshot.player.bounds.x,
			300.0f
		) &&
		nearlyEqual(
			slideSnapshot.player.bounds.y,
			212.0f
		) &&
		!neon::intersects(
			slideSnapshot.player.bounds,
			slideSnapshot.obstacles[0].bounds
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
		slideMovementPassed;

	return passed ? 0 : 1;
}