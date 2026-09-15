#include "core/TeleportDetector.h"

#include <algorithm>
#include <cmath>

namespace {
	bool finitePositon(
		const neon::Vec2& position
	) {
		return
			std::isfinite(position.x) &&
			std::isfinite(position.y);
	}

}//namespace

namespace neon {

	TeleportDetector::TeleportDetector(
		EntityId subjectId)
		: subjectId_(subjectId) {
	}

	std::optional<DetectionEvent>
		TeleportDetector::observe(
			std::uint64_t tick,
			Vec2 previousPostion,
			Vec2 currentPosition,
			float maxAllowedDistance) {

		if (!finitePositon(previousPostion) ||
			!finitePositon(currentPosition) ||
			!std::isfinite(maxAllowedDistance) ||
			maxAllowedDistance < 0.0f) {
			reset();
			return std::nullopt;
		}

		const float distance =
			std::hypot(
				currentPosition.x -
				previousPostion.x,
				currentPosition.y -
				previousPostion.y
			);

		if (!std::isfinite(distance)) {
			reset();
			return std::nullopt;
		}

		if (distance <= maxAllowedDistance) {
			reset();
			return std::nullopt;
		}

		if (reportedForJump_) {
			return std::nullopt;
		}

		DetectionEvent event{};
		event.tick = tick;
		event.type =
			DetectionType::TeleportAnomaly;
		event.subjectId = subjectId_;

		event.confidence =
			std::clamp(
				(distance - maxAllowedDistance) /
				std::max(maxAllowedDistance, 1.0f),
				0.0f,
				1.0f
			);

		event.evidence.observedValue =
			distance;
		event.evidence.expectedValue =
			maxAllowedDistance;
		event.evidence.deviation =
			distance - maxAllowedDistance;
		event.evidence.sampleCount = 1;
		event.evidence.position =
			currentPosition;

		reportedForJump_ = true;
		return event;
	}

	void TeleportDetector::reset() {
		reportedForJump_ = false;
	}

}//namespace neon