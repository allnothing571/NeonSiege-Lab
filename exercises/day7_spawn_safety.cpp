#include <algorithm>
#include <random>
#include <vector>

#include "core/GameplayConfig.h"
#include "core/SpawnPlacement.h"
#include "core/WaveManager.h"

namespace {

bool directSafetyZonePassed() {
	const neon::Rect worldBounds{
		0.0f,
		0.0f,
		300.0f,
		200.0f
	};

	const neon::Rect playerBounds{
		130.0f,
		80.0f,
		40.0f,
		40.0f
	};

	const std::vector<neon::Obstacle> obstacles;
	const std::vector<neon::Enemy> enemies;

	const neon::Rect insideSafetyZone{
		115.0f,
		80.0f,
		20.0f,
		20.0f
	};

	const neon::Rect outsideSafetyZone{
		60.0f,
		80.0f,
		20.0f,
		20.0f
	};

	return !neon::isSpawnAreaValid(
		insideSafetyZone,
		worldBounds,
		playerBounds,
		obstacles,
		enemies,
		20.0f
	) &&
		neon::isSpawnAreaValid(
			outsideSafetyZone,
			worldBounds,
			playerBounds,
			obstacles,
			enemies,
			20.0f
		);
}

bool waveManagerUsesSafetyMargin() {
	neon::GameplayConfig config{};
	config.maximumEnemiesPerWave = 6;
	config.spawnMaxAttemptsPerEnemy = 256;
	config.playerSpawnSafeMargin = 96.0f;

	const neon::Rect playerBounds{
		config.playerStartPosition.x,
		config.playerStartPosition.y,
		config.playerSize,
		config.playerSize
	};

	const std::vector<neon::Obstacle> obstacles;
	std::vector<neon::Enemy> enemies;
	const std::vector<neon::Enemy> noEnemies;
	neon::WaveManager waveManager;
	std::mt19937 randomEngine{ config.randomSeed };
	neon::EntityId nextEntityId = 1;

	const int spawned =
		waveManager.spawnNextWave(
			enemies,
			randomEngine,
			config,
			playerBounds,
			obstacles,
			nextEntityId
		);

	if (spawned != 3 ||
		static_cast<int>(enemies.size()) != spawned) {

		return false;
	}

	return std::all_of(
		enemies.begin(),
		enemies.end(),
		[&](const neon::Enemy& enemy) {
			return neon::isSpawnAreaValid(
				enemy.bounds(),
				config.worldBounds,
				playerBounds,
				obstacles,
				noEnemies,
				config.playerSpawnSafeMargin
			);
		}
	);
}

}

int main() {
	return directSafetyZonePassed() &&
		waveManagerUsesSafetyMargin()
		? 0
		: 1;
}
