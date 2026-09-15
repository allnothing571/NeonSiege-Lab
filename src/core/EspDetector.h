#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"
#include "core/ReplayFrame.h"

namespace neon {

	class EspDetector {
	public:
		EspDetector(
			EntityId subjectId,
			float angleToleranceDegrees = 1.0f,
			std::uint32_t requiredSamples = 3u
		);

		std::optional<DetectionEvent> observe(
			const ReplayFrame& frame,
			Vec2 playerCenter,
			Vec2 targetCenter,
			bool hasLineOfSight,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ =
			invalidEntityId;

		float angleToleranceDegrees_ =
			1.0f;

		std::uint32_t requiredSamples_ =
			3u;

		std::uint32_t suspiciousSamples_ =
			0u;

		bool reportedForStreak_ =
			false;
	};

}//namespace neon