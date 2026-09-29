#include "core/UpgradeSystem.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {

	std::size_t upgradeIndex(
		neon::UpgradeType type) noexcept {
		return static_cast<std::size_t>(type);
	}

	template <typename Value>
	Value takeRandom(
		std::vector<Value>& values,
		std::mt19937& randomEngine) {
		std::uniform_int_distribution<std::size_t> distribution(
			0,
			values.size() - 1
		);
		const std::size_t index = distribution(randomEngine);
		const Value result = values[index];
		values.erase(values.begin() + index);
		return result;
	}

}//namespace

namespace neon {

	void UpgradeSystem::reset() {
		levels_.fill(0);
		offers_.fill(UpgradeType::Count);
		offerCount_ = 0;
	}

	bool UpgradeSystem::generateOffers(
		std::mt19937& randomEngine) {
		std::vector<UpgradeType> offense;
		std::vector<UpgradeType> support;

		for (std::size_t index = 0;
			index < upgradeTypeCount;
			++index) {
			const auto type = static_cast<UpgradeType>(index);
			if (level(type) >= maximumLevel(type)) {
				continue;
			}

			(offensive(type) ? offense : support).push_back(type);
		}

		offers_.fill(UpgradeType::Count);
		offerCount_ = 0;

		if (offense.empty() && support.empty()) {
			return false;
		}

		if (!offense.empty()) {
			offers_[offerCount_++] =
				takeRandom(offense, randomEngine);
		}
		if (!support.empty()) {
			offers_[offerCount_++] =
				takeRandom(support, randomEngine);
		}

		std::vector<UpgradeType> remaining;
		remaining.reserve(offense.size() + support.size());
		remaining.insert(
			remaining.end(),
			offense.begin(),
			offense.end()
		);
		remaining.insert(
			remaining.end(),
			support.begin(),
			support.end()
		);

		while (offerCount_ < offerCapacity &&
			!remaining.empty()) {
			offers_[offerCount_++] =
				takeRandom(remaining, randomEngine);
		}

		if (offerCount_ != offerCapacity) {
			offers_.fill(UpgradeType::Count);
			offerCount_ = 0;
			return false;
		}
		std::shuffle(
			offers_.begin(),
			offers_.begin() + offerCount_,
			randomEngine
		);
		return true;
	}

	bool UpgradeSystem::applyOffer(
		int optionIndex) {
		if (optionIndex < 0 ||
			optionIndex >= offerCount_) {
			return false;
		}

		const UpgradeType type = offers_[optionIndex];
		if (type == UpgradeType::Count ||
			level(type) >= maximumLevel(type)) {
			return false;
		}

		++levels_[upgradeIndex(type)];
		offers_.fill(UpgradeType::Count);
		offerCount_ = 0;
		return true;
	}

	bool UpgradeSystem::hasOffers() const noexcept {
		return offerCount_ > 0;
	}

	int UpgradeSystem::offerCount() const noexcept {
		return offerCount_;
	}

	UpgradeType UpgradeSystem::offerAt(
		int optionIndex) const noexcept {
		if (optionIndex < 0 ||
			optionIndex >= offerCount_) {
			return UpgradeType::Count;
		}
		return offers_[optionIndex];
	}

	int UpgradeSystem::level(
		UpgradeType type) const noexcept {
		if (type == UpgradeType::Count) {
			return 0;
		}
		return levels_[upgradeIndex(type)];
	}

	int UpgradeSystem::maximumLevel(
		UpgradeType type) noexcept {
		switch (type) {
		case UpgradeType::HighVoltageRounds:
			return 1;
		case UpgradeType::FireRate:
		case UpgradeType::AmmoSystem:
		case UpgradeType::ArmorCore:
		case UpgradeType::BallisticCalibration:
			return 2;
		default:
			return 0;
		}
	}

	bool UpgradeSystem::offensive(
		UpgradeType type) noexcept {
		return type == UpgradeType::HighVoltageRounds ||
			type == UpgradeType::FireRate;
	}

	UpgradeStats UpgradeSystem::stats(
		const GameplayConfig& config) const noexcept {
		return statsForLevels(config, levels_);
	}

	UpgradeStats UpgradeSystem::statsAfter(
		const GameplayConfig& config,
		UpgradeType type) const noexcept {
		auto nextLevels = levels_;
		if (type != UpgradeType::Count) {
			const std::size_t index = upgradeIndex(type);
			nextLevels[index] = std::min(
				nextLevels[index] + 1,
				maximumLevel(type)
			);
		}
		return statsForLevels(config, nextLevels);
	}

	UpgradeStats UpgradeSystem::statsForLevels(
		const GameplayConfig& config,
		const std::array<int, upgradeTypeCount>& levels
	) const noexcept {
		const int highVoltageLevel = levels[
			upgradeIndex(UpgradeType::HighVoltageRounds)
		];
		const int fireRateLevel = levels[
			upgradeIndex(UpgradeType::FireRate)
		];
		const int ammoLevel = levels[
			upgradeIndex(UpgradeType::AmmoSystem)
		];
		const int armorLevel = levels[
			upgradeIndex(UpgradeType::ArmorCore)
		];
		const int ballisticLevel = levels[
			upgradeIndex(UpgradeType::BallisticCalibration)
		];

		UpgradeStats result{};
		result.projectileDamage =
			config.playerProjectileDamage +
			highVoltageLevel * 3;
		result.fireInterval = std::max(
			0.0f,
			config.fireInterval /
				(1.0f + 0.10f * fireRateLevel)
		);
		result.magazineCapacity = std::max(
			0,
			config.magazineCapacity + ammoLevel * 3
		);
		result.reloadDuration = std::max(
			0.0f,
			config.reloadDuration - ammoLevel * 0.10f
		);
		result.maximumHealth = std::max(
			0,
			config.playerInitialHealth + armorLevel * 15
		);
		result.projectileSpreadDegrees = std::max(
			0.0f,
			config.playerProjectileSpreadDegrees -
				ballisticLevel * 1.0f
		);
		result.projectileSpeed = std::max(
			0.0f,
			config.playerProjectileSpeed +
				ballisticLevel * 60.0f
		);
		return result;
	}

}//namespace neon
