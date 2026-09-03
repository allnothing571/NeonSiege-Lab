#pragma once

#include <cstdint>

namespace neon {

	using EntityId = std::uint32_t;

	constexpr EntityId invalidEntityId = 0;
	constexpr EntityId playerEntityId = 1;

}//namespace neon