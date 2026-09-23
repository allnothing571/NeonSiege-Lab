#pragma once

#include <optional>
#include <random>
#include <vector>

#include "core/MapDefinition.h"

namespace neon {

	const std::vector<MapDefinition>& presetMaps();

	const MapDefinition* findPresetMap(
		MapId id
	) noexcept;

	MapId chooseRandomPresetMap(
		std::mt19937& randomEngine,
		std::optional<MapId> previousMap = std::nullopt
	);

}//namespace neon
