#include <algorithm>
#include <random>
#include <vector>

#include "core/GameplayConfig.h"
#include "core/WaveManager.h"

int countShooters(
	const std::vector<neon::Enemy>& enemies) {

	return static_cast<int>(
		std::count_if(
			enemies.begin(),
			enemies.end(),
			[](const neon::Enemy& enemy) {
				return enemy.kind() ==
					neon::EnemyKind::Shooter;
			}
		)
		);
}

int main() {
	neon::GameplayConfig config{};
	config.spawnMaxAttemptsPerEnemy = 256;

	const neon::Rect playerBounds{
		config.playerStartPosition.x,
		config.playerStartPosition.y,
		config.playerSize,
		config.playerSize
	};

	const std::vector<neon::Obstacle> obstacles;

	std::vector<neon::Enemy> enemies;
	neon::WaveManager waveManager;
	std::mt19937 randomEngine{
		config.randomSeed
	};

	bool firstWavePassed = false;
	bool secondWavePassed = false;
	bool fourthWavePassed = false;
	bool seventhWavePassed = false;
	bool tenthWavePassed = false;

	for (int expectedWave = 1;
		expectedWave <= 10;
		++expectedWave) {

		enemies.clear();

		const int spawned =
			waveManager.spawnNextWave(
				enemies,
				randomEngine,
				config,
				playerBounds,
				obstacles
			);

		const int shooters =
			countShooters(enemies);

		if (expectedWave == 1) {
			firstWavePassed =
				spawned == 3 &&
				shooters == 0;
		}

		else if (expectedWave == 2) {
			secondWavePassed =
				spawned == 4 &&
				shooters == 1;
		}

		else if (expectedWave == 4) {
			fourthWavePassed =
				spawned == 6 &&
				shooters == 2;
		}

		else if (expectedWave == 7) {
			seventhWavePassed =
				spawned == 9 &&
				shooters == 3;
		}

		else if (expectedWave == 10) {
			tenthWavePassed =
				spawned == 12 &&
				shooters == 3;
		}
	}

	const bool passed =
		firstWavePassed &&
		secondWavePassed &&
		fourthWavePassed &&
		seventhWavePassed &&
		tenthWavePassed;

	return passed ? 0 : 1;
}