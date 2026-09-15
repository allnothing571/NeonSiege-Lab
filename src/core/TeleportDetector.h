#pragma once

#include <cstdint>
#include <optional>

#include "core/DetectionEvent.h"

namespace neon {

	class TeleportDetector {
	public:
		explicit TeleportDetector(
			EntityId subjectId
		);

		std::optional<DetectionEvent> observe(
			std::uint64_t tick,
			Vec2 previousPosition,
			Vec2 currentPosition,
			float maAllowedDistance
		);

		void reset();

	private:
		EntityId subjectId_ =
			invalidEntityId;

		bool reportedForJump_ = false;
	};
}