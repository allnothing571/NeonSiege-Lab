#include <iostream>
#include <vector>
#include <cmath>
#include "core/WaveManager.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;
	return std::fabs(first - second) <= epsilon;
}

int main() {
	std::vector<neon::Enemy> enemies;
	neon::WaveManager waveManager;

	const bool initialStatePassed =
		waveManager.currentWave() == 0 &&
		enemies.empty();

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	waveManager.spawnNextWave(enemies);

	const bool firstWavePassed =
		waveManager.currentWave() == 1 &&
		enemies.size() == 3;

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	enemies.clear();

	waveManager.spawnNextWave(enemies);

	const bool secondWavePassed =
		waveManager.currentWave() == 2 &&
		enemies.size() == 4;

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	enemies.clear();

	waveManager.spawnNextWave(enemies);

	const bool thirdWavePassed =
		waveManager.currentWave() == 3 &&
		enemies.size() == 5 &&
		nearlyEqual(enemies.back().bounds().x, 80.0f) &&
		nearlyEqual(enemies.back().bounds().y, 220.f);

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	std::cout << "lastenemyX: " << enemies.back().bounds().x << ' '
		<< "lastenemyY: " << enemies.back().bounds().y << '\n';

	waveManager.reset();

	const bool resetWavePassed =
		waveManager.currentWave() == 0;

	enemies.clear();

	waveManager.spawnNextWave(enemies);

	const bool firstWaveAfterPassed =
		waveManager.currentWave() == 1 &&
		enemies.size() == 3;

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	const bool passed =
		initialStatePassed &&
		firstWavePassed &&
		secondWavePassed &&
		thirdWavePassed &&
		resetWavePassed &&
		firstWaveAfterPassed;

	return passed ? 0 : 1;
}