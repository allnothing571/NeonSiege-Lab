#include <iostream>
#include "core/Player.h"

int main() {
	neon::Rect worldBounds = { 100.0f,50.0f,200.0f,100.0f };
	neon::Vec2 PlayerPosition = { 120.0f,70.0f };
	neon::Player player = { PlayerPosition, 20.0f, 100.0f, 3 };
	float dt = 0.5f;
	neon::Vec2 direction = { 1.0f, 0.0f };

	player.update(
		direction,
		dt,
		worldBounds
	);

	neon::Rect bounds = player.bounds();
	neon::Vec2 center = player.center();

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
	std::cout << "center().x: " << center.x << ' ' << "center().y: " << center.y << '\n';

	player.takeDamage(5);

	int health = player.health();
	int isAlive = player.isAlive();

	std::cout << "health(): " << health << ' ' << "isAlive(): " << isAlive << '\n';

	PlayerPosition = { 280.0f, 130.0f };
	player.reset(PlayerPosition, 3);
	direction = { 1.0f, 1.0f };

	player.update(
		direction,
		dt,
		worldBounds
	);
	bounds = player.bounds();

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
}