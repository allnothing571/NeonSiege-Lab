#include <cstdint>
#include <vector>

#include "core/DamageDetector.h"

namespace {

	neon::GameEvent makeDamage(
		std::uint64_t tick,
		neon::GameEntityKind sourceKind,
		neon::EntityId sourceId,
		neon::GameEntityKind targetKind,
		neon::EntityId targetId,
		int damage
	) {
		neon::GameEvent event{};
		event.tick = tick;
		event.type =
			neon::GameEventType::DamageApplied;
		event.sourceKind = sourceKind;
		event.sourceId = sourceId;
		event.targetKind = targetKind;
		event.targetId = targetId;
		event.value = damage;
		event.position = { 200.0f, 100.0f };
		return event;
	}

}//namespace

int main() {
	neon::DamageDetector detector(
		neon::playerEntityId,
		10
	);

	const auto legitimate =
		detector.observe(
			1,
			{
				makeDamage(
					1,
					neon::GameEntityKind::Projectile,
					2u,
					neon::GameEntityKind::Enemy,
					7u,
					10
				)
			}
		);

	if (legitimate.has_value()) {
		return 1;
	}

	const auto excessive =
		detector.observe(
			2,
			{
				makeDamage(
					2,
					neon::GameEntityKind::Projectile,
					2u,
					neon::GameEntityKind::Enemy,
					7u,
					999
				)
			}
		);

	if (!excessive.has_value() ||
		excessive->type !=
		neon::DetectionType::DamageAnomaly ||
		excessive->subjectId !=
		neon::playerEntityId ||
		excessive->confidence != 1.0f ||
		excessive->evidence.observedValue !=
		999.0f ||
		excessive->evidence.expectedValue !=
		10.0f) {

		return 2;
	}

	const auto duplicate =
		detector.observe(
			2,
			{
				makeDamage(
					2,
					neon::GameEntityKind::Projectile,
					3u,
					neon::GameEntityKind::Enemy,
					8u,
					500
				)
			}
		);

	if (duplicate.has_value()) {
		return 3;
	}

	detector.reset();

	const auto invalidSource =
		detector.observe(
			3,
			{
				makeDamage(
					3,
					neon::GameEntityKind::Player,
					neon::playerEntityId,
					neon::GameEntityKind::Enemy,
					7u,
					10
				)
			}
		);

	if (!invalidSource.has_value()) {
		return 4;
	}

	detector.reset();

	const auto unrelatedDamage =
		detector.observe(
			4,
			{
				makeDamage(
					4,
					neon::GameEntityKind::Projectile,
					4u,
					neon::GameEntityKind::Player,
					neon::playerEntityId,
					10
				)
			}
		);

	if (unrelatedDamage.has_value()) {
		return 5;
	}

	return 0;
}