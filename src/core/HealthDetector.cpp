#include "core/HealthDetector.h"

#include <algorithm>

namespace neon {

	HealthDetector::HealthDetector(
		EntityId subjectId,
		int maxHealth)
		: subjectId_(subjectId),
		maxHealth_(std::max(1, maxHealth)) {
	}

	std::optional<DetectionEvent>
		HealthDetector::observe(
			std::uint64_t tick,
			int previousHealth,
			int currentHealth,
			const std::vector<GameEvent>& events) {

		std::int64_t totalDamage = 0;

		for (const GameEvent& event : events) {
			if (event.tick != tick ||
				event.type != GameEventType::DamageApplied ||
				event.targetKind != GameEntityKind::Player ||
				event.targetId != subjectId_ ||
				event.value <= 0) {

				continue;
			}

			totalDamage = std::min(
				totalDamage +
				static_cast<std::int64_t>(event.value),
				static_cast<std::int64_t>(maxHealth_)
			);
		}

		const std::int64_t expectedRaw =
			static_cast<std::int64_t>(previousHealth) -
			totalDamage;

		const int expectedHealth =
			static_cast<int>(
				std::clamp<std::int64_t>(
					expectedRaw,
					0,
					static_cast<std::int64_t>(maxHealth_)
				)
				);

		const bool rangeAnomaly =
			previousHealth < 0 ||
			previousHealth > maxHealth_ ||
			currentHealth < 0 ||
			currentHealth > maxHealth_;

		const bool healthMismatch =
			currentHealth != expectedHealth;

		if (!rangeAnomaly && !healthMismatch) {
			reset();
			return std::nullopt;
		}

		if (reportedForState_) {
			return std::nullopt;
		}

		DetectionEvent detection{};
		detection.tick = tick;
		detection.type = DetectionType::HealthAnomaly;
		detection.subjectId = subjectId_;
		detection.confidence = 1.0f;

		const std::int64_t difference =
			static_cast<std::int64_t>(currentHealth) -
			static_cast<std::int64_t>(expectedHealth);

		detection.evidence.observedValue =
			static_cast<float>(currentHealth);
		detection.evidence.expectedValue =
			static_cast<float>(expectedHealth);

		detection.evidence.deviation =
			static_cast<float>(
				difference < 0 ? -difference : difference
				);

		detection.evidence.sampleCount = 1;

		reportedForState_ = true;
		return detection;
	}

	void HealthDetector::reset() {
		reportedForState_ = false;
	}

}//namespace neon