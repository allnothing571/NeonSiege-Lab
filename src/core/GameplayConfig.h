#pragma once

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
		int playerInitialHealth = 3;
		int enemyContactDamage = 1;

		float playerProjectileSize = 8.0f;
		float playerProjectileSpeed = 600.0f;
		int playerProjectileDamage = 10;

		int scorePerEnemy = 100;
	};

}//namespace neon