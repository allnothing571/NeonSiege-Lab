#include <cstdint>
#include <vector>

#include "core/ScoreDetector.h"

namespace {

	neon::GameEvent makeProjectileHit(
		std::uint64_t tick,
		neon::EntityId projectileId,
		neon::EntityId enemyId
	) {
		neon::GameEvent event{};
		event.tick = tick;
		event.type =
			neon::GameEventType::ProjectileHit;
		event.sourceKind =
			neon::GameEntityKind::Projectile;
		event.sourceId = projectileId;
		event.targetKind =
			neon::GameEntityKind::Enemy;
		event.targetId = enemyId;
		event.value = 10;
		event.position = { 200.0f, 100.0f };
		return event;
	}

	neon::GameEvent makeEnemyDeath(
		std::uint64_t tick,
		neon::EntityId enemyId
	) {
		neon::GameEvent event{};
		event.tick = tick;
		event.type =
			neon::GameEventType::EntityDied;
		event.sourceKind =
			neon::GameEntityKind::Enemy;
		event.sourceId = enemyId;
		event.position = { 200.0f, 100.0f };
		return event;
	}

}//namespace

int main() {
	neon::ScoreDetector detector(
		neon::playerEntityId,
		100
	);

	const auto legitimate =
		detector.observe(
			1,
			0,
			100,
			{
				makeProjectileHit(1, 2u, 7u),
				makeEnemyDeath(1, 7u)
			}
		);

	if (legitimate.has_value()) {
		return 1;
	}

	const auto forged =
		detector.observe(
			2,
			100,
			500,
			{}
		);

	if (!forged.has_value() ||
		forged->type !=
		neon::DetectionType::ScoreAnomaly ||
		forged->subjectId !=
		neon::playerEntityId ||
		forged->confidence != 1.0f ||
		forged->evidence.observedValue !=
		500.0f ||
		forged->evidence.expectedValue !=
		100.0f) {

		return 2;
	}

	if (detector.observe(
		2,
		100,
		500,
		{}
	).has_value()) {

		return 3;
	}

	detector.reset();

	const auto inconsistent =
		detector.observe(
			3,
			100,
			250,
			{
				makeProjectileHit(3, 3u, 8u),
				makeEnemyDeath(3, 8u)
			}
		);

	if (!inconsistent.has_value() ||
		inconsistent->evidence.expectedValue !=
		200.0f) {

		return 4;
	}

	detector.reset();

	const auto unpairedDeath =
		detector.observe(
			4,
			200,
			200,
			{
				makeEnemyDeath(4, 9u)
			}
		);

	if (unpairedDeath.has_value()) {
		return 5;
	}

	detector.reset();

	const auto scoreDecrease =
		detector.observe(
			5,
			200,
			100,
			{}
		);

	if (!scoreDecrease.has_value()) {
		return 6;
	}

	return 0;
}