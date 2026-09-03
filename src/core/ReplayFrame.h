#pragma once

#include <cstdint>

#include "core/InputCommand.h"

namespace neon {

	struct  ReplayFrame
	{
		std::uint64_t tick = 0;
		InputCommand command{};
	};

}//namespace neon