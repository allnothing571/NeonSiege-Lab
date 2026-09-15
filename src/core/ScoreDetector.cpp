#include "core/ScoreDetector.h"

#include <algorithm>
#include <limits>

namespace {

	bool hasProjectileHitForEnemy(
		const std::vector<neon::GameEvent>& events,
		std::uint64_t tick,
		neon::EntityId enemyId
	) {
		for (const neon::GameEvent& event : events) {
			if (event.tick != tick ||
				event.type !=
				neon::GameEventType::ProjectileHit ||
				event.sourceKind !=
				neon::GameEntityKind::Projectile ||
				event.sourceId ==
				neon::invalidEntityId ||
				event.targetKind !=
				neon::GameEntityKind::Enemy ||
				event.targetId != enemyId) {

				continue;
			}

			return true;
		}

		return false;
	}

}//namespace

namespace neon {

	ScoreDetector::ScoreDetector(
		EntityId subjectId,
		int scorePerEnemy)
		: subjectId_(subjectId),
		scorePerEnemy_(
			std::max(0, scorePerEnemy)) {
	}

	std::optional<DetectionEvent>
		ScoreDetector::observe(
			std::uint64_t tick,
			int previousScore,
			int currentScore,
			const std::vector<GameEvent>& events
		) {
		std::int64_t validKillCount = 0;
		Vec2 evidencePosition{};

		for (const GameEvent& event : events) {
			if (event.tick != tick ||
				event.type !=
				GameEventType::EntityDied ||
				event.sourceKind !=
				GameEntityKind::Enemy ||
				event.sourceId ==
				invalidEntityId) {

				continue;
			}

			if (!hasProjectileHitForEnemy(
				events,
				tick,
				event.sourceId)) {

				continue;
			}

			++validKillCount;
			evidencePosition = event.position;
		}

		const std::int64_t expectedScore =
			static_cast<std::int64_t>(previousScore) +
			validKillCount *
			static_cast<std::int64_t>(
				scorePerEnemy_);

		const bool scoreOutOfRange =
			previousScore < 0 ||
			currentScore < 0;

		const bool scoreMismatch =
			static_cast<std::int64_t>(currentScore) !=
			expectedScore;

		if (!scoreOutOfRange &&
			!scoreMismatch) {

			reset();
			return std::nullopt;
		}

		if (reportedForState_) {
			return std::nullopt;
		}

		DetectionEvent detection{};
		detection.tick = tick;
		detection.type =
			DetectionType::ScoreAnomaly;
		detection.subjectId = subjectId_;
		detection.confidence = 1.0f;

		detection.evidence.observedValue =
			static_cast<float>(currentScore);

		detection.evidence.expectedValue =
			static_cast<float>(expectedScore);

		const std::int64_t difference =
			static_cast<std::int64_t>(currentScore) -
			expectedScore;

		detection.evidence.deviation =
			static_cast<float>(
				difference < 0
				? -difference
				: difference
				);

		const std::int64_t maxSampleCount =
			static_cast<std::int64_t>(
				std::numeric_limits<
				std::uint32_t
				>::max()
				);

		detection.evidence.sampleCount =
			static_cast<std::uint32_t>(
				std::min(
					validKillCount,
					maxSampleCount
				)
				);

		detection.evidence.position =
			evidencePosition;

		reportedForState_ = true;
		return detection;
	}

	void ScoreDetector::reset() {
		reportedForState_ = false;
	}

}//namespace neon