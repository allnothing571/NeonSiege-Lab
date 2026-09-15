#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "core/DetectionEvent.h"
#include "core/GameEvent.h"

namespace neon {

	class HealthDetector {
	public:
		HealthDetector(
			EntityId subjectId,
			int maxHealth
		);

		std::optional<DetectionEvent> observe(
			std::uint64_t tick,
			int previousHealth,
			int currentHealth,
			const std::vector<GameEvent>& events
		);

		void reset();

	private:
		EntityId subjectId_ = invalidEntityId;
		int maxHealth_ = 1;
		bool reportedForState_ = false;
	};

}//namespace neon