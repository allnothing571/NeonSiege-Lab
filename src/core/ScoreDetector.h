#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"

namespace neon {

	class ScoreDetector {
	public:
		ScoreDetector(
			EntityId subjectId,
			int scorePerEnemy
		);

		std::optional<DetectionEvent> observe(
			std::uint64_t tick,
			int previousScore,
			int currentScore,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ = invalidEntityId;
		int scorePerEnemy_ = 0;

		bool reportedForState_ = false;
	};

}//namespace neon