#include <cmath>
#include <vector>

#include "core/Collision.h"
#include "core/Enemy.h"
#include "core/GameplayConfig.h"

namespace {

bool insideWorld(
	const neon::Rect& bounds,
	const neon::Rect& worldBounds) {
	return bounds.x >= worldBounds.x &&
		bounds.y >= worldBounds.y &&
		bounds.x + bounds.w <=
		worldBounds.x + worldBounds.w &&
		bounds.y + bounds.h <=
		worldBounds.y + worldBounds.h;
}

bool avoidsHorizontalWall(neon::EnemyKind kind) {
	const neon::Rect worldBounds{ 0.0f, 0.0f, 640.0f, 480.0f };
	const neon::Obstacle wall{
		neon::Rect{ 100.0f, 100.0f, 360.0f, 24.0f }
	};
	const std::vector<neon::Obstacle> obstacles{ wall };

	neon::Enemy enemy{
		neon::Vec2{ 260.0f, 50.0f },
		20.0f,
		80.0f,
		30,
		kind,
		180.0f,
		320.0f,
		0.3f,
		1.2f,
		kind == neon::EnemyKind::Chaser ? 2u : 3u
	};

	const neon::Vec2 start = enemy.center();
	const neon::Vec2 target{ 270.0f, 430.0f };

	for (int step = 0; step < 180; ++step) {
		enemy.update(target, 0.05f, worldBounds, obstacles);

		if (!insideWorld(enemy.bounds(), worldBounds) ||
			neon::intersects(enemy.bounds(), wall.bounds())) {
			return false;
		}
	}

	const neon::Vec2 end = enemy.center();
	const neon::Rect wallBounds = wall.bounds();
	const bool crossedWall =
		end.y > wallBounds.y + wallBounds.h +
			enemy.bounds().h / 2.0f;

	return crossedWall &&
		end.y > start.y;
}

bool avoidsVerticalWall() {
	const neon::Rect worldBounds{ 0.0f, 0.0f, 640.0f, 480.0f };
	const neon::Obstacle wall{
		neon::Rect{ 100.0f, 60.0f, 24.0f, 300.0f }
	};
	const std::vector<neon::Obstacle> obstacles{ wall };

	neon::Enemy enemy{
		neon::Vec2{ 50.0f, 180.0f },
		20.0f,
		80.0f,
		30,
		neon::EnemyKind::Chaser,
		180.0f,
		320.0f,
		0.3f,
		1.2f,
		4u
	};

	const neon::Vec2 start = enemy.center();
	const neon::Vec2 target{ 500.0f, 190.0f };

	for (int step = 0; step < 180; ++step) {
		enemy.update(target, 0.05f, worldBounds, obstacles);

		if (!insideWorld(enemy.bounds(), worldBounds) ||
			neon::intersects(enemy.bounds(), wall.bounds())) {
			return false;
		}
	}

	const neon::Vec2 end = enemy.center();
	const neon::Rect wallBounds = wall.bounds();
	const bool crossedWall =
		end.x > wallBounds.x + wallBounds.w +
			enemy.bounds().w / 2.0f;

	return crossedWall &&
		end.x > start.x;
}

}

int main() {
	const bool chaserPassed =
		avoidsHorizontalWall(neon::EnemyKind::Chaser);
	const bool shooterPassed =
		avoidsHorizontalWall(neon::EnemyKind::Shooter);
	const bool verticalWallPassed =
		avoidsVerticalWall();

	return chaserPassed &&
		shooterPassed &&
		verticalWallPassed
		? 0
		: 1;
}
