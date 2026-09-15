#pragma once

#include <algorithm>
#include <random>
#include <vector>

#include "core/Enemy.h"
#include "core/GameplayConfig.h"
#include "core/Obstacle.h"
#include "core/SpawnPlacement.h"

namespace neon {

	class WaveManager {
	public:
		void reset() {
			currentWave_ = 0;
		}

		int spawnNextWave(
			std::vector<Enemy>& enemies,
			std::mt19937& randomEngine,
			const GameplayConfig& config,
			const Rect& playerBounds,
			const std::vector<Obstacle>& obstacles,
			EntityId& nextEntityId) {

			const int nextWave = currentWave_ + 1;

			const int enemyCount =
				std::max(
					0,
					std::min(
						nextWave + 2,
						config.maximumEnemiesPerWave
					)
				);

			const int shooterCount =
				nextWave == 1
				? 0
				: std::min(
					3,
					enemyCount / 3
				);

			int spawnedCount = 0;

			for (int index = 0;
				index < enemyCount;
				++index) {

				const auto position =
					findRandomSpawnPosition(
						randomEngine,
						config.worldBounds,
						config.enemySize,
						playerBounds,
						obstacles,
						enemies,
						config.spawnMaxAttemptsPerEnemy,
						config.playerSpawnSafeMargin
					);

				if (!position.has_value()) {
					continue;
				}

				const float speed =
					config.enemyBaseSpeed +
					static_cast<float>(
						index % 3
						) * config.enemySpeedStep;

				const EnemyKind kind =
					spawnedCount < shooterCount
					? EnemyKind::Shooter
					: EnemyKind::Chaser;

				const int health =
					kind == EnemyKind::Shooter
					? config.shooterHealth
					: config.enemyHealth;

				enemies.emplace_back(
					*position,
					config.enemySize,
					speed,
					health,
					kind,
					config.shooterRetreatDistance,
					config.shooterApproachDistance,
					config.shooterWarningDuration,
					config.shooterFireInterval,
					nextEntityId++
				);

				++spawnedCount;
			}

			if (spawnedCount > 0) {
				currentWave_ = nextWave;
			}

			return spawnedCount;
		}

		int currentWave() const {
			return currentWave_;
		}

	private:
		int currentWave_ = 0;
	};
}
