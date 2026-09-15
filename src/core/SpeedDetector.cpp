#include "core/SpeedDetector.h"

#include <algorithm>
#include <cmath>

namespace {

	bool finitePosition(
		const neon::Vec2& position
	) {
		return
			std::isfinite(position.x) &&
			std::isfinite(position.y);
	}

	float nonNegativeOr(
		float value,
		float fallback
	) {
		return
			std::isfinite(value) &&
			value >= 0.0f
			? value
			: fallback;
	}

}//namespace

namespace neon {

	SpeedDetector::SpeedDetector(
		EntityId subjectId,
		float expectedSpeed,
		float relativeTolerance,
		std::uint32_t requiredSamples)
		: subjectId_(subjectId),
		expectedSpeed_(
			nonNegativeOr(expectedSpeed, 0.0f)),
		relativeTolerance_(
			nonNegativeOr(relativeTolerance, 0.0f)),
		requiredSamples_(
			std::max(1u, requiredSamples)) {
	}

	std::optional<DetectionEvent>
		SpeedDetector::observe(
			std::uint64_t tick,
			Vec2 previousPosition,
			Vec2 currentPosition,
			float fixedDt) {

		if (!finitePosition(previousPosition) ||
			!finitePosition(currentPosition) ||
			!std::isfinite(fixedDt) ||
			fixedDt <= 0.0f) {
			reset();
			return std::nullopt;
		}

		const float deltaX =
			currentPosition.x -
			previousPosition.x;

		const float deltaY =
			currentPosition.y -
			previousPosition.y;

		const float distance =
			std::hypot(deltaX, deltaY);

		const float observedSpeed =
			distance / fixedDt;

		if (!std::isfinite(observedSpeed)) {
			reset();
			return std::nullopt;
		}

		const float allowedSpeed =
			expectedSpeed_ *
			(1.0f + relativeTolerance_) +
			0.01f;

		if (observedSpeed <= allowedSpeed) {
			reset();
			return std::nullopt;
		}

		if (suspiciousSamples_ <
			requiredSamples_) {
			++suspiciousSamples_;
		}

		if (reportedForStreak_ ||
			suspiciousSamples_ <
			requiredSamples_) {
			return std::nullopt;
		}

		const float confidence =
			std::clamp(
				(observedSpeed - allowedSpeed) /
				std::max(expectedSpeed_, 1.0f),
				0.0f,
				1.0f
			);

		DetectionEvent event{};
		event.tick = tick;
		event.type =
			DetectionType::SpeedAnomaly;
		event.subjectId = subjectId_;
		event.confidence = confidence;

		event.evidence.observedValue =
			observedSpeed;
		event.evidence.expectedValue =
			expectedSpeed_;
		event.evidence.deviation =
			observedSpeed - expectedSpeed_;
		event.evidence.sampleCount =
			suspiciousSamples_;
		event.evidence.position =
			currentPosition;

		reportedForStreak_ = true;
		return event;
	}

	void SpeedDetector::reset() {
		suspiciousSamples_ = 0u;
		reportedForStreak_ = false;
	}

}//namespace neon