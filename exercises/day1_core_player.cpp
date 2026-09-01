#include <iostream>
#include <cmath>
#include "core/Player.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

int main() {
	neon::Rect worldBounds = { 100.0f,50.0f,200.0f,100.0f };
	neon::Vec2 playerPosition = { 120.0f,70.0f };
	neon::Player player = { playerPosition, 20.0f, 100.0f, 3, 0.5f };
	float dt = 0.5f;
	neon::Vec2 direction = { 1.0f, 0.0f };

	player.update(
		direction,
		dt,
		worldBounds
	);

	neon::Rect bounds = player.bounds();
	neon::Vec2 center = player.center();

	const bool movementPassed =
		nearlyEqual(bounds.x, 170.0f) &&
		nearlyEqual(bounds.y, 70.0f) &&
		nearlyEqual(center.x, 180.0f) &&
		nearlyEqual(center.y, 80.0f);

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
	std::cout << "center().x: " << center.x << ' ' << "center().y: " << center.y << '\n';

	player.takeDamage(5);

	const bool damagePassed =
		player.health() == 0 &&
		!player.isAlive();

	int health = player.health();
	int isAlive = player.isAlive();

	std::cout << "health(): " << health << ' ' << "isAlive(): " << isAlive << '\n';

	playerPosition = { 120.0f, 130.0f };
	player.reset(playerPosition, 3);
	direction = { 1.0f, 1.0f };

	player.update(
		direction,
		dt,
		worldBounds
	);
	bounds = player.bounds();

	const bool resetPassed =
		player.health() == 3 &&
		player.isAlive();

	const bool edgeMovementPassed =
		nearlyEqual(bounds.x, 170.0f) &&
		nearlyEqual(bounds.y, 130.0f);

	player.reset({ 280.0f, 130.0f }, 3);
	direction = { 1.0f, 1.0f };

	player.update(
		direction,
		dt,
		worldBounds
	);

	bounds = player.bounds();

	const bool boundaryPassed =
		nearlyEqual(bounds.x, 280.0f) &&
		nearlyEqual(bounds.y, 130.0f);

	const bool passed =
		movementPassed &&
		damagePassed &&
		resetPassed &&
		edgeMovementPassed &&
		boundaryPassed;

	return passed ? 0 : 1;
}