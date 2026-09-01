#include "core/SpawnPlacement.h"

#include <algorithm>

#include "core/Collision.h"

namespace neon {

	bool isSpawnAreaValid(
		const Rect& candidate,
		const Rect& worldBounds,
		const Rect& playerBounds,
		const std::vector<Obstacle>& obstacles,
		const std::vector<Enemy>& enemies) {

		const bool insideWorld =
			candidate.x >= worldBounds.x &&
			candidate.y >= worldBounds.y &&
			candidate.x + candidate.w <=
			worldBounds.x + worldBounds.w &&
			candidate.y + candidate.h <=
			worldBounds.y + worldBounds.h;

		if (!insideWorld ||
			intersects(candidate, playerBounds)) {
			return false;
		}

		const bool touchesObstacle =
			std::any_of(
				obstacles.begin(),
				obstacles.end(),
				[&candidate](const Obstacle& obstacle) {
					return intersects(
						candidate,
						obstacle.bounds()
					);
				}
			);

		if (touchesObstacle) {
			return false;
		}

		return std::none_of(
			enemies.begin(),
			enemies.end(),
			[&candidate](const Enemy& enemy) {
				return intersects(
					candidate,
					enemy.bounds()
				);
			}
		);
	}

	std::optional<Vec2> findRandomSpawnPosition(
		std::mt19937& randomEngine,
		const Rect& worldBounds,
		float enemySize,
		const Rect& playerBounds,
		const std::vector<Obstacle>& obstacles,
		const std::vector<Enemy>& enemies,
		int maxAttemps) {

		if (enemySize <= 0.0f ||
			maxAttemps <= 0 ||
			worldBounds.w < enemySize ||
			worldBounds.h < enemySize) {
			return std::nullopt;
		}

		const float maximumX =
			worldBounds.x + worldBounds.w - enemySize;

		const float maximumY =
			worldBounds.y + worldBounds.h - enemySize;

		std::uniform_real_distribution<float> xDistribution(
			worldBounds.x,
			maximumX
		);

		std::uniform_real_distribution<float> yDistribution(
			worldBounds.y,
			maximumY
		);

		for (int attempt = 0;
			attempt < maxAttemps;
			++attempt) {

			const Vec2 position{
				xDistribution(randomEngine),
				yDistribution(randomEngine)
			};

			const Rect candidate{
				position.x,
				position.y,
				enemySize,
				enemySize
			};

			if (isSpawnAreaValid(
				candidate,
				worldBounds,
				playerBounds,
				obstacles,
				enemies)) {

				return position;
			}
		}

		return std::nullopt;
	}

}//namespace neon