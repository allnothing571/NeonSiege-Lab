#include <iostream>
#include <cmath>
#include "core/Enemy.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

int main() {
	neon::Vec2 position{ 0.0f, 0.0f };
	neon::Vec2 target{ 35.0f, 5.0f };
	float size = 10.0f;
	float speed = 100.0f;
	int health = 30;
	float dt = 0.2f;

	neon::Enemy enemy1(position, size, speed, health);

	enemy1.update(target, dt);

	const bool movementPassed =
		nearlyEqual(enemy1.bounds().x, 20.0f) &&
		nearlyEqual(enemy1.bounds().y, 0.0f);

	std::cout << "bounds().x: " << enemy1.bounds().x << ' '
		<< "bounds().y: " << enemy1.bounds().y << '\n';

	enemy1.update(enemy1.center(), dt);

	const bool zeroDirectionPassed =
		nearlyEqual(enemy1.bounds().x, 20.0f) &&
		nearlyEqual(enemy1.bounds().y, 0.0f);

	const bool hitboxPassed =
		nearlyEqual(enemy1.hitbox().x, 22.0f) &&
		nearlyEqual(enemy1.hitbox().y, 2.0f) &&
		nearlyEqual(enemy1.hitbox().w, 6.0f) &&
		nearlyEqual(enemy1.hitbox().h, 6.0f);

	std::cout << "bounds().x: " << enemy1.bounds().x << ' '
		<< "bounds().y: " << enemy1.bounds().y << '\n';

	std::cout << "hitbox().x: " << enemy1.hitbox().x << ' '
		<< "hitbox().y: " << enemy1.hitbox().y << ' '
		<< "hitbox().w: " << enemy1.hitbox().w << ' '
		<< "hitbox().h: " << enemy1.hitbox().h << '\n';

	enemy1.takeDamage(-5);
	const bool negativeDamagePassed = enemy1.isAlive();

	std::cout << "isAlive(): " << enemy1.isAlive() << '\n';

	enemy1.takeDamage(30);

	const bool lethalDamagePassed = !enemy1.isAlive();

	std::cout << "isAlive(): " << enemy1.isAlive() << '\n';

	neon::Enemy enemy2{ position, size, speed, health };

	enemy2.defeat();

	const bool defeatPassed = !enemy2.isAlive();

	std::cout << "isAlive(): " << enemy2.isAlive() << '\n';

	const bool passed =
		movementPassed &&
		zeroDirectionPassed &&
		hitboxPassed &&
		negativeDamagePassed &&
		lethalDamagePassed &&
		defeatPassed;

	return passed ? 0 : 1;
}