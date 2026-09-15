#include "core/GameplayConfig.h"

int main() {
	neon::GameplayConfig config{};

	const bool defaultsPassed =
		config.enemyContactDamage == 10 &&
		config.worldBounds.x == 0.0f &&
		config.worldBounds.y == 0.0f &&
		config.worldBounds.w == 960.0f &&
		config.worldBounds.h == 540.0f &&
		config.playerStartPosition.x == 456.0f &&
		config.playerStartPosition.y == 246.0f &&
		config.playerSize == 48.0f &&
		config.playerSpeed == 240.0f &&
		config.playerInitialHealth == 100 &&
		config.playerProjectileSize == 8.0f &&
		config.playerProjectileSpeed == 600.0f &&
		config.playerProjectileSpreadDegrees == 3.0f &&
		config.playerProjectileDamage == 10 &&
		config.playerInvulnerabilityDuration == 0.5f &&
		config.scorePerEnemy == 100 &&
		config.enemySize == 32.0f &&
		config.enemyBaseSpeed == 70.0f &&
		config.enemySpeedStep == 10.0f &&
		config.enemyHealth == 30 &&
		config.maximumEnemiesPerWave == 12 &&
		config.maximumWaves == 10 &&
		config.spawnMaxAttemptsPerEnemy == 64 &&
		config.playerSpawnSafeMargin == 96.0f &&
		config.randomSeed == 1337u &&
		config.magazineCapacity == 12 &&
		config.fireInterval == 0.2f &&
		config.reloadDuration == 1.2f;

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
