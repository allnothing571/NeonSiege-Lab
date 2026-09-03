#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "core/GameplayConfig.h"
#include "core/WaveManager.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

int main() {
	const neon::GameplayConfig config{};

	const neon::Rect playerBounds{
		config.playerStartPosition.x,
		config.playerStartPosition.y,
		config.playerSize,
		config.playerSize
	};

	const std::vector<neon::Obstacle> obstacles;

	std::vector<neon::Enemy> enemies;
	neon::WaveManager waveManager;
	std::mt19937 randomEngine{ config.randomSeed };
	neon::EntityId nextEntityId = 1;

	const bool initialStatePassed =
		waveManager.currentWave() == 0 &&
		enemies.empty();

	const int firstSpawned =
		waveManager.spawnNextWave(
			enemies,
			randomEngine,
			config,
			playerBounds,
			obstacles,
			nextEntityId
		);

	const neon::Rect firstPosition =
		enemies.front().bounds();

	const bool firstWavePassed =
		firstSpawned == 3 &&
		waveManager.currentWave() == 1 &&
		enemies.size() == 3;

	enemies.clear();

	const int secondSpawned =
		waveManager.spawnNextWave(
			enemies,
			randomEngine,
			config,
			playerBounds,
			obstacles,
			nextEntityId
		);

	const bool secondWavePassed =
		secondSpawned == 4 &&
		waveManager.currentWave() == 2 &&
		enemies.size() == 4;

	enemies.clear();

	const int thirdSpawned =
		waveManager.spawnNextWave(
			enemies,
			randomEngine,
			config,
			playerBounds,
			obstacles,
			nextEntityId
		);

	const bool thirdWavePassed =
		thirdSpawned == 5 &&
		waveManager.currentWave() == 3 &&
		enemies.size() == 5;

	waveManager.reset();
	randomEngine.seed(config.randomSeed);
	enemies.clear();

	const int resetSpawned =
		waveManager.spawnNextWave(
			enemies,
			randomEngine,
			config,
			playerBounds,
			obstacles,
			nextEntityId
		);

	const neon::Rect resetFirstPosition =
		enemies.front().bounds();

	const bool resetAndDeterminismPassed =
		resetSpawned == 3 &&
		waveManager.currentWave() == 1 &&
		enemies.size() == 3 &&
		nearlyEqual(
			firstPosition.x,
			resetFirstPosition.x
		) &&
		nearlyEqual(
			firstPosition.y,
			resetFirstPosition.y
		);

	const bool passed =
		initialStatePassed &&
		firstWavePassed &&
		secondWavePassed &&
		thirdWavePassed &&
		resetAndDeterminismPassed;

	std::cout
		<< "first: " << firstSpawned << '\n'
		<< "second: " << secondSpawned << '\n'
		<< "third: " << thirdSpawned << '\n'
		<< "reset: " << resetSpawned << '\n';

	return passed ? 0 : 1;
}