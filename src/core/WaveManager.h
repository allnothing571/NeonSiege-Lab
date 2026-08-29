#pragma once

#include <vector>
#include "core/Enemy.h"

namespace neon {

	class WaveManager {
	public:
		void reset() {
			currentWave_ = 0;
		}

		void spawnNextWave(std::vector<neon::Enemy>& enemies) {
			++currentWave_;

			const int enemyCount = currentWave_ + 2;

			for (int i = 0; i < enemyCount; i++) {
				const neon::Vec2 position = {
					80.0f + static_cast<float>(i % 4) * 220.0f,
					80.0f + static_cast<float>(i / 4) * 140.0f,
				};
				const float speed =
					70.0f + static_cast<float>(i % 3) * 10.0f;

				enemies.emplace_back(
					position,
					32.0f,
					speed,
					30);
			}
		}

		int currentWave() const {
			return currentWave_;
		}

	private:
		int currentWave_ = 0;
	};
}