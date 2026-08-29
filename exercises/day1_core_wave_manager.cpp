#include <iostream>
#include <vector>
#include "core/WaveManager.h"

int main() {
	std::vector<neon::Enemy> enemies;
	neon::WaveManager waveManager;

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	waveManager.spawnNextWave(enemies);

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	enemies.clear();

	waveManager.spawnNextWave(enemies);

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	enemies.clear();

	waveManager.spawnNextWave(enemies);

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';

	std::cout << "lastenemyX: " << enemies.back().bounds().x << ' '
		<< "lastenemyY: " << enemies.back().bounds().y << '\n';

	waveManager.reset();

	enemies.clear();

	waveManager.spawnNextWave(enemies);

	std::cout << "currentWave: " << waveManager.currentWave() << ' '
		<< "enemiesCount: " << enemies.size() << '\n';
}