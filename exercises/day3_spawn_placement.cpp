#include <cmath>
#include <random>
#include <vector>

#include "core/SpawnPlacement.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;

	return std::fabs(first - second) <= epsilon;
}

int main() {
	const neon::Rect worldBounds{
		0.0f,
		0.0f,
		300.0f,
		200.0f
	};

	const neon::Rect playerBounds{
		130.0f,
		80.0f,
		40.0f,
		40.0f
	};

	std::vector<neon::Obstacle> obstacles;

	obstacles.emplace_back(
		neon::Rect{
			40.0f,
			40.0f,
			60.0f,
			60.0f
		}
	);

	std::vector<neon::Enemy> enemies;

	enemies.emplace_back(
		neon::Vec2{ 220.0f, 120.0f },
		20.0f,
		0.0f,
		1
	);

	std::mt19937 firstEngine{ 1337u };
	std::mt19937 secondEngine{ 1337u };

	const auto firstPosition =
		neon::findRandomSpawnPosition(
			firstEngine,
			worldBounds,
			20.0f,
			playerBounds,
			obstacles,
			enemies,
			64
		);

	const auto secondPosition =
		neon::findRandomSpawnPosition(
			secondEngine,
			worldBounds,
			20.0f,
			playerBounds,
			obstacles,
			enemies,
			64
		);

	const bool deterministicPassed =
		firstPosition.has_value() &&
		secondPosition.has_value() &&
		nearlyEqual(
			firstPosition->x,
			secondPosition->x
		) &&
		nearlyEqual(
			firstPosition->y,
			secondPosition->y
		);

	const bool validPositionPassed =
		firstPosition.has_value() &&
		neon::isSpawnAreaValid(
			neon::Rect{
				firstPosition->x,
				firstPosition->y,
				20.0f,
				20.0f
			},
			worldBounds,
			playerBounds,
			obstacles,
			enemies
		);

	std::vector<neon::Obstacle> blockedObstacles;

	blockedObstacles.emplace_back(worldBounds);

	const std::vector<neon::Enemy> noEnemies;

	std::mt19937 blockedEngine{ 42u };

	const auto failedPosition =
		neon::findRandomSpawnPosition(
			blockedEngine,
			worldBounds,
			20.0f,
			playerBounds,
			blockedObstacles,
			noEnemies,
			4
		);

	const bool failurePassed =
		!failedPosition.has_value();

	return deterministicPassed &&
		validPositionPassed &&
		failurePassed
		? 0
		: 1;
}