#pragma once

#include <array>
#include <random>

#include "core/GameplayConfig.h"
#include "core/UpgradeType.h"

namespace neon {

	struct UpgradeStats {
		int projectileDamage = 0;
		float fireInterval = 0.0f;
		int magazineCapacity = 0;
		float reloadDuration = 0.0f;
		int maximumHealth = 0;
		float projectileSpreadDegrees = 0.0f;
		float projectileSpeed = 0.0f;
	};

	class UpgradeSystem {
	public:
		static constexpr int offerCapacity = 3;

		void reset();
		bool generateOffers(std::mt19937& randomEngine);
		bool applyOffer(int optionIndex);

		bool hasOffers() const noexcept;
		int offerCount() const noexcept;
		UpgradeType offerAt(int optionIndex) const noexcept;

		int level(UpgradeType type) const noexcept;
		static int maximumLevel(UpgradeType type) noexcept;
		static bool offensive(UpgradeType type) noexcept;

		UpgradeStats stats(
			const GameplayConfig& config) const noexcept;
		UpgradeStats statsAfter(
			const GameplayConfig& config,
			UpgradeType type) const noexcept;

	private:
		UpgradeStats statsForLevels(
			const GameplayConfig& config,
			const std::array<int, upgradeTypeCount>& levels
		) const noexcept;

		std::array<int, upgradeTypeCount> levels_{};
		std::array<UpgradeType, offerCapacity> offers_{
			UpgradeType::Count,
			UpgradeType::Count,
			UpgradeType::Count
		};
		int offerCount_ = 0;
	};

}//namespace neon
