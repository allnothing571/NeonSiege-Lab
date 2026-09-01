#include <iostream>
#include <cmath>
#include "core/Projectile.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

int main() {
	neon::Vec2 position = { 120.0f, 70.0f };
	neon::Rect worldBounds = { 100.0f, 50.0f, 200.0f, 100.0f };
	neon::Vec2 direction = { 1.0f, 0.0f };
	float size = 5.0f;
	float speed = 100.0f;

	neon::Projectile projectile(
		position,
		direction,
		size,
		speed,
		neon::ProjectileFaction::Player,
		10,
		5.0f
	);

	projectile.update(0.5f);
	projectile.update(0.5f);

	neon::Rect bounds = projectile.bounds();

	const bool movementPassed =
		nearlyEqual(bounds.x, 220.0f) &&
		nearlyEqual(bounds.y, 70.0f) &&
		!projectile.isOutside(worldBounds);

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
	std::cout << "isOutside(worldBounds): " << projectile.isOutside(worldBounds) << '\n';

	projectile.update(1.0f);

	bounds = projectile.bounds();

	const bool outsidePassed =
		nearlyEqual(bounds.x, 320.0f) &&
		nearlyEqual(bounds.y, 70.0f) &&
		projectile.isOutside(worldBounds);

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
	std::cout << "isOutside(worldBounds): " << projectile.isOutside(worldBounds) << '\n';

	std::cout << "isConsumed(): " << projectile.isConsumed() << '\n';

	const bool notConsumedBefore =
		!projectile.isConsumed();

	projectile.consume();

	std::cout << "isConsumed(): " << projectile.isConsumed() << '\n';

	const bool consumedAfter =
		projectile.isConsumed();

	const bool passed =
		movementPassed &&
		outsidePassed &&
		notConsumedBefore &&
		consumedAfter;

	return passed ? 0 : 1;
}