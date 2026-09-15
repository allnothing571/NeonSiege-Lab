#include "core/DamageDetector.h"

#include <algorithm>

namespace neon {

	DamageDetector::DamageDetector(
		EntityId subjectId,
		int maxAllowedDamage)
		: subjectId_(subjectId),
		maxAllowedDamage_(
			std::max(1, maxAllowedDamage)) {
	}

	std::optional<DetectionEvent>
		DamageDetector::observe(
			std::uint64_t tick,
			const std::vector<GameEvent>& events) {

		for (const GameEvent& event : events) {
			if (event.tick != tick ||
				event.type != GameEventType::DamageApplied ||
				event.targetKind != GameEntityKind::Enemy) {

				continue;
			}

			const bool invalidSource =
				event.sourceKind !=
				GameEntityKind::Projectile ||
				event.sourceId ==
				invalidEntityId;

			const bool invalidTarget =
				event.targetId ==
				invalidEntityId;

			const bool invalidAmount =
				event.value <= 0 ||
				event.value > maxAllowedDamage_;

			if (!invalidSource &&
				!invalidTarget &&
				!invalidAmount) {

				continue;
			}

			if (hasReportedTick_ &&
				reportedTick_ == tick) {

				return std::nullopt;
			}

			DetectionEvent detection{};
			detection.tick = tick;
			detection.type =
				DetectionType::DamageAnomaly;
			detection.subjectId = subjectId_;
			detection.confidence = 1.0f;

			detection.evidence.observedValue =
				static_cast<float>(event.value);

			detection.evidence.expectedValue =
				invalidAmount
				? static_cast<float>(maxAllowedDamage_)
				: 0.0f;

			const std::int64_t expectedValue =
				invalidAmount
				? static_cast<std::int64_t>(
					maxAllowedDamage_)
				: 0;

			const std::int64_t difference =
				static_cast<std::int64_t>(event.value) -
				expectedValue;

			detection.evidence.deviation =
				static_cast<float>(
					difference < 0
					? -difference
					: difference
					);

			detection.evidence.sampleCount = 1;
			detection.evidence.position =
				event.position;

			hasReportedTick_ = true;
			reportedTick_ = tick;

			return detection;
		}

		return std::nullopt;
	}

	void DamageDetector::reset() {
		hasReportedTick_ = false;
		reportedTick_ = 0;
	}

}//namespace neon