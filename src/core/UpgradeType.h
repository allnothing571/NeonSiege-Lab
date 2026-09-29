#pragma once

#include <cstddef>

namespace neon {

	enum class UpgradeType {
		HighVoltageRounds,
		FireRate,
		AmmoSystem,
		ArmorCore,
		BallisticCalibration,
		Count
	};

	constexpr std::size_t upgradeTypeCount =
		static_cast<std::size_t>(UpgradeType::Count);

}//namespace neon
