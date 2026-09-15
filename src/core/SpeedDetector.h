#pragma once

#include <cstdint>
#include <optional>

#include "core/DetectionEvent.h"

namespace neon {

	class SpeedDetector {
	public:
		SpeedDetector(
			EntityId subjectId,
			float expectedSpeed,
			float relativeTolerance = 0.10f,
			std::uint32_t requiredSamples = 3u
		);

		std::optional<DetectionEvent> observe(
			std::uint64_t tick,
			Vec2 previousPosition,
			Vec2 currentPosition,
			float fixedDt
		);

		void reset();

	private:
		EntityId subjectId_ =
			invalidEntityId;

		float expectedSpeed_ = 0.0f;
		float relativeTolerance_ = 0.10f;

		std::uint32_t requiredSamples_ = 3u;
		std::uint32_t suspiciousSamples_ = 0u;

		bool reportedForStreak_ = false;
	};
}