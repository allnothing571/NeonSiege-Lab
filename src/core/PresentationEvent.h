#pragma once

#include <cstdint>

#include "core/EntityId.h"
#include "core/Types.h"

namespace neon {

	enum class PresentationEventType {
		None,
		PlayerDamaged,
		EnemyDamaged,
		ProjectileHit,
		EnemyDied
	};

	struct PresentationEvent
	{
		std::uint64_t tick = 0;
		PresentationEventType type = PresentationEventType::None;
		EntityId sourceId = invalidEntityId;
		EntityId targetId = invalidEntityId;
		Vec2 position{};
		int value = 0;
	};

}//namespace neon
