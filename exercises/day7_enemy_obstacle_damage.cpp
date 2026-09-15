#include <cmath>
#include <vector>

#include "core/Enemy.h"
#include "core/GameplayConfig.h"

namespace {

bool nearlyEqual(float first, float second) {
	return std::fabs(first - second) <= 0.001f;
}

bool blockedByWall(
	neon::EnemyKind kind,
	const neon::Rect& worldBounds) {
	std::vector<neon::Obstacle> obstacles;
	obstacles.emplace_back(
		neon::Rect{ 40.0f, -50.0f, 20.0f, 200.0f }
	);

	neon::Enemy enemy{
		neon::Vec2{ 0.0f, 0.0f },
		32.0f,
		100.0f,
		30,
		kind
	};

	enemy.update(
		neon::Vec2{ 500.0f, 16.0f },
		0.5f,
		worldBounds,
		obstacles
	);

	const neon::Rect result = enemy.bounds();
	const bool movedAroundWall =
		std::fabs(result.x) > 0.001f ||
		std::fabs(result.y) > 0.001f;
	const bool stillOutsideWall =
		!neon::intersects(result, obstacles.front().bounds());

	return movedAroundWall && stillOutsideWall;
}

}

int main() {
	const neon::GameplayConfig config{};

	const bool chaserBlocked =
		blockedByWall(
			neon::EnemyKind::Chaser,
			config.worldBounds
		);
	const bool shooterBlocked =
		blockedByWall(
			neon::EnemyKind::Shooter,
			config.worldBounds
		);
	const bool contactDamageConfigured =
		config.enemyContactDamage == 10;

	return chaserBlocked &&
		shooterBlocked &&
		contactDamageConfigured
		? 0
		: 1;
}
