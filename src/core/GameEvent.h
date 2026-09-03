#pragma once

#include "core/Types.h"
#include "core/EntityId.h"

namespace neon {

	enum class GameEntityKind {
		None,
		Player,
		Enemy,
		Projectile,
		Obstacle
	};

	enum class GameEventType {
		None,
		PlayerShot,
		EnemyShot,
		ProjectileHit,
		DamageApplied,
		EntityDied,
		ReloadStarted,
		ReloadCompleted,
		WaveStarted,
		WaveCompleted,
		GameStateChanged
	};

	struct GameEvent
	{
		std::uint64_t tick = 0;

		GameEventType type =
			GameEventType::None;

		GameEntityKind sourceKind =
			GameEntityKind::None;

		EntityId sourceId =
			invalidEntityId;

		GameEntityKind targetKind =
			GameEntityKind::None;

		EntityId targetId =
			invalidEntityId;

		int value = 0;
		Vec2 position{};
	};

}//namespace neon