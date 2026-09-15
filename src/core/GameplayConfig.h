#pragma once

#include <cstdint>

#include "core/Types.h"

namespace neon {

	struct GameplayConfig {
		Rect worldBounds{
			0.0f,
			0.0f,
			960.0f,
			540.0f
		};

		Vec2 playerStartPosition{
			456.0f,
			246.0f
		};

		float playerSize = 48.0f;
		float playerSpeed = 240.0f;
		int playerInitialHealth = 100;
		float playerInvulnerabilityDuration = 0.5f;

		int enemyContactDamage = 10;
		float enemySize = 32.0f;
		float enemyBaseSpeed = 70.0f;
		float enemySpeedStep = 10.0f;
		int enemyHealth = 30;

		float shooterRetreatDistance = 180.0f;
		float shooterApproachDistance = 320.0f;
		int shooterHealth = 20;
		float shooterWarningDuration = 0.3f;
		float shooterFireInterval = 1.2f;

		int maximumEnemiesPerWave = 12;
		int maximumWaves = 10;
		float waveIntermissionDuration = 2.0f;
		int spawnMaxAttemptsPerEnemy = 64;
		float playerSpawnSafeMargin = 96.0f;

		std::uint32_t randomSeed = 1337u;

		int magazineCapacity = 12;
		float fireInterval = 0.2f;
		float reloadDuration = 1.2f;

		float playerProjectileSize = 8.0f;
		float playerProjectileSpeed = 600.0f;
		float playerProjectileSpreadDegrees = 3.0f;
		int playerProjectileDamage = 10;
		float playerProjectileLifetime = 5.0f;

		float enemyProjectileSize = 10.0f;
		float enemyProjectileSpeed = 240.0f;
		int enemyProjectileDamage = 10;
		float enemyProjectileLifetime = 5.0f;
		int maximumEnemyProjectiles = 64;

		int scorePerEnemy = 100;
	};

}//namespace neon
