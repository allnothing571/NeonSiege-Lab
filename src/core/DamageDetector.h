#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"

namespace neon {

	class DamageDetector {
	public:
		DamageDetector(
			EntityId subjectId,
			int maxAllowedDamage
		);

		std::optional<DetectionEvent> observe(
			std::uint64_t tick,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ = invalidEntityId;
		int maxAllowedDamage_ = 1;

		bool hasReportedTick_ = false;
		std::uint64_t reportedTick_ = 0;
	};

}//namespace neon
