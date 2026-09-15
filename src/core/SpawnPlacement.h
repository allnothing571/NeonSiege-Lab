#pragma once

#include <optional>
#include <random>
#include <vector>

#include "core/Enemy.h"
#include "core/Obstacle.h"

namespace neon {

	bool isSpawnAreaValid(
		const Rect& candidate,
		const Rect& worldBounds,
		const Rect& playerBounds,
		const std::vector<Obstacle>& obstacles,
		const std::vector<Enemy>& enemies,
		float playerSpawnSafeMargin = 0.0f
	);

	std::optional<Vec2> findRandomSpawnPosition(
		std::mt19937& randomEngine,
		const Rect& worldBounds,
		float enemySize,
		const Rect& playerBounds,
		const std::vector<Obstacle>& obstacles,
		const std::vector<Enemy>& enemies,
		int maxAttempts,
		float playerSpawnSafeMargin = 0.0f
	);

}//namespace neon
