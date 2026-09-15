#include "core/AutoFireDetector.h"

#include <algorithm>

namespace neon {

	AutoFireDetector::AutoFireDetector(
		EntityId subjectId,
		std::uint32_t requiredSamples)
		: subjectId_(subjectId),
		requiredSamples_(
			std::max(1u, requiredSamples)) {
	}

	std::optional<DetectionEvent>
		AutoFireDetector::observe(
			const ReplayFrame& frame,
			const std::vector<GameEvent>& events) {

		const GameEvent* unexpectedShot = nullptr;

		for (const GameEvent& event : events) {
			if (event.tick == frame.tick &&
				event.type == GameEventType::PlayerShot &&
				event.sourceKind == GameEntityKind::Player &&
				event.sourceId == subjectId_) {

				unexpectedShot = &event;
				break;
			}
		}

		if (unexpectedShot == nullptr ||
			frame.command.fireHeld) {

			reset();
			return std::nullopt;
		}

		if (suspiciousSamples_ < requiredSamples_) {
			++suspiciousSamples_;
		}

		if (reportedForStreak_ ||
			suspiciousSamples_ < requiredSamples_) {

			return std::nullopt;
		}

		DetectionEvent detection{};
		detection.tick = frame.tick;
		detection.type = DetectionType::AutoFire;
		detection.subjectId = subjectId_;
		detection.confidence = 1.0f;

		detection.evidence.observedValue = 1.0f;
		detection.evidence.expectedValue = 0.0f;
		detection.evidence.deviation = 1.0f;
		detection.evidence.sampleCount = suspiciousSamples_;
		detection.evidence.position = unexpectedShot->position;

		reportedForStreak_ = true;
		return detection;
	}

	void AutoFireDetector::reset() {
		suspiciousSamples_ = 0;
		reportedForStreak_ = false;
	}

}//namespace neon