#include <cmath>
#include <vector>

#include "core/Enemy.h"
#include "core/GameplayConfig.h"

namespace {

bool nearlyEqual(float first, float second) {
	return std::fabs(first - second) <= 0.001f;
}

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

bool rightBoundaryStopsShooter(
	const neon::Rect& worldBounds) {

	constexpr float size = 32.0f;
	constexpr float speed = 100.0f;
	constexpr int health = 20;
	constexpr float startY = 200.0f;

	neon::Enemy shooter{
		neon::Vec2{
			worldBounds.x + worldBounds.w - size,
			startY
		},
		size,
		speed,
		health,
		neon::EnemyKind::Shooter
	};

	shooter.update(
		neon::Vec2{
			worldBounds.x + worldBounds.w + 500.0f,
			startY + size / 2.0f
		},
		1.0f,
		worldBounds
	);

	return nearlyEqual(
		shooter.bounds().x,
		worldBounds.x + worldBounds.w - size
	) &&
		nearlyEqual(shooter.bounds().y, startY) &&
		insideWorld(shooter.bounds(), worldBounds);
}

bool bottomBoundaryStopsShooter(
	const neon::Rect& worldBounds) {

	constexpr float size = 32.0f;
	constexpr float speed = 100.0f;
	constexpr int health = 20;
	constexpr float startX = 400.0f;

	neon::Enemy shooter{
		neon::Vec2{
			startX,
			worldBounds.y + worldBounds.h - size
		},
		size,
		speed,
		health,
		neon::EnemyKind::Shooter
	};

	shooter.update(
		neon::Vec2{
			startX + size / 2.0f,
			worldBounds.y + worldBounds.h + 500.0f
		},
		1.0f,
		worldBounds
	);

	return nearlyEqual(shooter.bounds().x, startX) &&
		nearlyEqual(
			shooter.bounds().y,
			worldBounds.y + worldBounds.h - size
		) &&
		insideWorld(shooter.bounds(), worldBounds);
}

bool blockedAxisStillSlides(
	const neon::Rect& worldBounds) {

	constexpr float size = 32.0f;
	constexpr float speed = 100.0f;
	constexpr int health = 20;
	constexpr float startY = 200.0f;

	const neon::Vec2 startPosition{
		worldBounds.x + worldBounds.w - size,
		startY
	};

	neon::Enemy shooter{
		startPosition,
		size,
		speed,
		health,
		neon::EnemyKind::Shooter
	};

	const neon::Vec2 target{
		worldBounds.x + worldBounds.w + 500.0f,
		startY + size / 2.0f + 500.0f
	};

	const std::vector<neon::Obstacle> obstacles;
	shooter.update(target, 1.0f, worldBounds, obstacles);

	const float directionX = target.x - (startPosition.x + size / 2.0f);
	const float directionY = target.y - (startPosition.y + size / 2.0f);
	const float directionLength =
		std::sqrt(directionX * directionX + directionY * directionY);
	const float expectedY =
		startY + speed * directionY / directionLength;

	return nearlyEqual(shooter.bounds().x, startPosition.x) &&
		nearlyEqual(shooter.bounds().y, expectedY) &&
		insideWorld(shooter.bounds(), worldBounds);
}

bool leftBoundaryStopsChaser(
	const neon::Rect& worldBounds) {

	constexpr float size = 32.0f;
	constexpr float speed = 100.0f;
	constexpr int health = 30;
	constexpr float startY = 200.0f;

	neon::Enemy chaser{
		neon::Vec2{ worldBounds.x, startY },
		size,
		speed,
		health
	};

	chaser.update(
		neon::Vec2{
			worldBounds.x - 500.0f,
			startY + size / 2.0f
		},
		1.0f,
		worldBounds
	);

	return nearlyEqual(chaser.bounds().x, worldBounds.x) &&
		nearlyEqual(chaser.bounds().y, startY) &&
		insideWorld(chaser.bounds(), worldBounds);
}

}

int main() {
	const neon::GameplayConfig config{};

	const bool passed =
		rightBoundaryStopsShooter(config.worldBounds) &&
		bottomBoundaryStopsShooter(config.worldBounds) &&
		blockedAxisStillSlides(config.worldBounds) &&
		leftBoundaryStopsChaser(config.worldBounds);

	return passed ? 0 : 1;
}
