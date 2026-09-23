#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "core/Types.h"

namespace neon {

	enum class MapId : std::uint32_t {
		Legacy = 0u,
		Crossfire = 1u,
		SplitCorridors = 2u,
		BrokenDistrict = 3u
	};

	struct MapDefinition {
		MapId id = MapId::Crossfire;
		std::string_view key{};
		Vec2 playerStartPosition{};
		std::vector<Rect> obstacleBounds{};
	};

}//namespace neon
