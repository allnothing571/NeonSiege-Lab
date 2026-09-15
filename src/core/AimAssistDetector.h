#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"
#include "core/ReplayFrame.h"

namespace neon {

	class AimAssistDetector {
	public:
		AimAssistDetector(
			EntityId subjectId,
			float angleToleranceDegrees = 0.5f,
			std::uint32_t requiredSamples = 5u
		);

		std::optional<DetectionEvent> observe(
			const ReplayFrame& frame,
			Vec2 playerCenter,
			Vec2 targetCenter,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ = invalidEntityId;

		float angleToleranceDegrees_ = 0.5f;

		std::uint32_t requiredSamples_ = 5u;
		std::uint32_t suspiciousSamples_ = 0u;

		bool reportedForStreak_ = false;
	};

}//namespace neon
