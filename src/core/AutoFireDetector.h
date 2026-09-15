#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"
#include "core/ReplayFrame.h"

namespace neon {

	class AutoFireDetector {
	public:
		AutoFireDetector(
			EntityId subjectId,
			std::uint32_t requiredSample = 2u
		);

		std::optional<DetectionEvent> observe(
			const ReplayFrame& frame,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ =
			invalidEntityId;

		std::uint32_t requiredSamples_ = 2u;
		std::uint32_t suspiciousSamples_ = 0u;

		bool reportedForStreak_ = false;
	};

}//namespace neon