#pragma once

#include <algorithm>

namespace neon {

	constexpr int waveDifficultyTier(
		int wave) noexcept {
		return std::max(0, (wave - 1) / 2);
	}

	constexpr int scaledEnemyHealth(
		int baseHealth,
		int wave) noexcept {
		if (baseHealth <= 0) {
			return 0;
		}
		const int tier = waveDifficultyTier(wave);
		return baseHealth +
			(baseHealth * tier + 5) / 10;
	}

	constexpr int scaledEnemyDamage(
		int baseDamage,
		int wave) noexcept {
		if (baseDamage <= 0) {
			return 0;
		}
		return baseDamage + waveDifficultyTier(wave);
	}

}//namespace neon
