#include <iostream>
#include "core/Projectile.h"

int main() {
	neon::Vec2 position = { 120.0f, 70.0f };
	neon::Rect worldBounds = { 100.0f, 50.0f, 200.0f, 100.0f };
	neon::Vec2 direction = { 1.0f, 0.0f };
	float size = 5.0f;
	float speed = 100.0f;

	neon::Projectile projectile(position, direction, size, speed);

	projectile.update(0.5f);
	projectile.update(0.5f);

	neon::Rect bounds = projectile.bounds();

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
	std::cout << "isOutside(worldBounds): " << projectile.isOutside(worldBounds) << '\n';

	projectile.update(1.0f);

	bounds = projectile.bounds();

	std::cout << "bounds().x: " << bounds.x << ' ' << "bounds().y: " << bounds.y << '\n';
	std::cout << "isOutside(worldBounds): " << projectile.isOutside(worldBounds) << '\n';

	std::cout << "isConsumed(): " << projectile.isConsumed() << '\n';

	projectile.consume();

	std::cout << "isConsumed(): " << projectile.isConsumed() << '\n';
}