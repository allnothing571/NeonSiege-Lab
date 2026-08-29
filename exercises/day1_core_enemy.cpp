#include <iostream>
#include "core/Enemy.h"

int main() {
	neon::Vec2 position{ 0.0f, 0.0f };
	neon::Vec2 target{ 35.0f, 5.0f };
	float size = 10.0f;
	float speed = 100.0f;
	int health = 30;
	float dt = 0.2f;

	neon::Enemy enemy1(position, size, speed, health);

	enemy1.update(target, dt);

	std::cout << "bounds().x: " << enemy1.bounds().x << ' '
		<< "bounds().y: " << enemy1.bounds().y << '\n';

	enemy1.update(enemy1.center(), dt);

	std::cout << "bounds().x: " << enemy1.bounds().x << ' '
		<< "bounds().y: " << enemy1.bounds().y << '\n';

	std::cout << "hitbox().x: " << enemy1.hitbox().x << ' '
		<< "hitbox().y: " << enemy1.hitbox().y << ' '
		<< "hitbox().w: " << enemy1.hitbox().w << ' '
		<< "hitbox().h: " << enemy1.hitbox().h << '\n';

	enemy1.takeDamage(-5);

	std::cout << "isAlive(): " << enemy1.isAlive() << '\n';

	enemy1.takeDamage(1000);

	std::cout << "isAlive(): " << enemy1.isAlive() << '\n';

	neon::Enemy enemy2{ position, size, speed, health };

	enemy2.defeat();

	std::cout << "isAlive(): " << enemy1.isAlive() << '\n';

	return 0;
}