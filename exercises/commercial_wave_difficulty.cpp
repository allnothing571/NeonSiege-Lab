#include <array>

#include "core/WaveDifficulty.h"

int main() {
	constexpr std::array<int, 10> chaserHealth{
		30, 30, 33, 33, 36, 36, 39, 39, 42, 42
	};
	constexpr std::array<int, 10> shooterHealth{
		20, 20, 22, 22, 24, 24, 26, 26, 28, 28
	};
	constexpr std::array<int, 10> damage{
		10, 10, 11, 11, 12, 12, 13, 13, 14, 14
	};

	for (int wave = 1; wave <= 10; ++wave) {
		const std::size_t index =
			static_cast<std::size_t>(wave - 1);
		if (neon::scaledEnemyHealth(30, wave) !=
			chaserHealth[index]) {
			return 1;
		}
		if (neon::scaledEnemyHealth(20, wave) !=
			shooterHealth[index]) {
			return 2;
		}
		if (neon::scaledEnemyDamage(10, wave) !=
			damage[index]) {
			return 3;
		}
	}

	if (neon::waveDifficultyTier(0) != 0 ||
		neon::scaledEnemyDamage(-5, 10) != 0 ||
		neon::scaledEnemyDamage(0, 10) != 0 ||
		neon::scaledEnemyHealth(0, 10) != 0) {
		return 4;
	}

	return 0;
}
