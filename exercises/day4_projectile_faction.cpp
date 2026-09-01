#include "core/Projectile.h"

int main() {
	neon::Projectile projectile{
		neon::Vec2{ 0.0f, 0.0f },
		neon::Vec2{ 1.0f, 0.0f },
		8.0f,
		240.0f,
		neon::ProjectileFaction::Enemy,
		10,
		0.5f
	};

	const bool propertiesPassed =
		projectile.faction() ==
		neon::ProjectileFaction::Enemy &&
		projectile.damage() == 10 &&
		!projectile.isConsumed();

	projectile.update(0.25f);

	const bool activeBeforeExpiry =
		!projectile.isConsumed();

	projectile.update(0.25f);

	const bool consumedAfterExpiry =
		projectile.isConsumed();

	return propertiesPassed &&
		activeBeforeExpiry &&
		consumedAfterExpiry
		? 0
		: 1;
}