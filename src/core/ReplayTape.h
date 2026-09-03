#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

#include "core/ReplayFrame.h"

namespace neon {

	struct ReplayTape
	{
		static constexpr std::uint32_t currentVersion = 1u;

		std::uint32_t version = currentVersion;
		std::uint32_t randomSeed = 0u;
		float fixedDt = 0.0f;
		std::vector<ReplayFrame> frames{};
	};

	bool writeReplay(
		std::ostream& output,
		const ReplayTape& tape,
		std::string& error
	);

	bool readReplay(
		std::istream& input,
		ReplayTape& tape,
		std::string& error
	);

}//namespace neon