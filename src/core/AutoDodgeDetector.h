#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"
#include "core/ReplayFrame.h"

namespace neon {

	class AutoDodgeDetector {
	public:
		AutoDodgeDetector(
			EntityId subjectId,
			float maxReactionTime = 0.15f,
			float maxThreatTime = 0.35f,
			float minimumLateralComponent = 0.8f,
			std::uint32_t requiredSamples = 3u
		);

		std::optional<DetectionEvent> observe(
			const ReplayFrame& frame,
			Vec2 projectileVelocity,
			float timeToImpact,
			float fixedDt,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ =
			invalidEntityId;

		float maxReactionTime_ =
			0.15f;

		float maxThreatTime_ =
			0.35f;

		float minimumLateralComponent_ =
			0.8f;

		std::uint32_t requiredSamples_ =
			3u;

		std::uint32_t suspiciousSamples_ =
			0u;

		bool reportedForStreak_ =
			false;

		bool pendingThreat_ =
			false;

		std::uint64_t threatStartTick_ =
			0u;

		Vec2 threatDirection_{};
		Vec2 threatPosition_{};

		Vec2 previousMovement_{};

		bool hasPreviousMovement_ =
			false;

		void resetStreak();
	};

}//namespace neon