#include "core/GameplayConfig.h"

int main() {
	neon::GameplayConfig config{};

	const bool defaultsPassed =
		config.enemyContactDamage == 1 &&
		config.worldBounds.x == 0.0f &&
		config.worldBounds.y == 0.0f &&
		config.worldBounds.w == 960.0f &&
		config.worldBounds.h == 540.0f &&
		config.playerStartPosition.x == 456.0f &&
		config.playerStartPosition.y == 246.0f &&
		config.playerSize == 48.0f &&
		config.playerSpeed == 240.0f &&
		config.playerInitialHealth == 3 &&
		config.playerProjectileSize == 8.0f &&
		config.playerProjectileSpeed == 600.0f &&
		config.playerProjectileDamage == 10 &&
		config.scorePerEnemy == 100;

	config.worldBounds.w = 1280.0f;
	config.playerSpeed = 300.0f;
	config.scorePerEnemy = 150;

	const bool overridesPassed =
		config.worldBounds.w == 1280.0f &&
		config.playerSpeed == 300.0f &&
		config.scorePerEnemy == 150;

	return defaultsPassed && overridesPassed
		? 0
		: 1;
}