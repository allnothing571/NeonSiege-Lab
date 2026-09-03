#include "core/GameEvent.h"

int main() {
	const neon::GameEvent damageEvent{
		120,
		neon::GameEventType::DamageApplied,
		neon::GameEntityKind::Projectile,
		42,
		neon::GameEntityKind::Player,
		1,
		10,
		{480.0f, 270.0f}
	};

	const bool damageEventPassed =
		damageEvent.tick == 120 &&
		damageEvent.type ==
		neon::GameEventType::DamageApplied &&
		damageEvent.sourceKind ==
		neon::GameEntityKind::Projectile &&
		damageEvent.sourceId == 42 &&
		damageEvent.targetKind ==
		neon::GameEntityKind::Player &&
		damageEvent.targetId == 1 &&
		damageEvent.value == 10 &&
		damageEvent.position.x == 480.0f &&
		damageEvent.position.y == 270.0f;

	const neon::GameEvent defaultEvent{};

	const bool defaultsPassed =
		defaultEvent.tick == 0 &&
		defaultEvent.type ==
		neon::GameEventType::None &&
		defaultEvent.sourceId ==
		neon::invalidEntityId &&
		defaultEvent.targetId ==
		neon::invalidEntityId;

	return damageEventPassed &&
		defaultsPassed
		? 0
		: 1;
}