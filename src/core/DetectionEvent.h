#pragma once

#include <cstdint>

#include "core/EntityId.h"
#include "core/Types.h"

namespace neon {

	enum class DetectionType {
		None,
		AimAssist,
		AutoFire,
		SpeedAnomaly,
		TeleportAnomaly,
		HealthAnomaly,
		DamageAnomaly,
		ScoreAnomaly,
		EspAnomaly,
		AutoDodge
	};

	struct DetectionEvidence
	{
		float observedValue = 0.0f;
		float expectedValue = 0.0f;
		float deviation = 0.0f;

		std::uint32_t sampleCount = 0;

		Vec2 position{};
	};

	struct DetectionEvent
	{
		std::uint64_t tick = 0;

		DetectionType type =
			DetectionType::None;

		EntityId subjectId =
			invalidEntityId;

		float confidence = 0.0f;

		DetectionEvidence evidence{};
	};

}//namespace neon