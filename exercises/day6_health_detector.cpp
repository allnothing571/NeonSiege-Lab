#include <cstdint>
#include <vector>

#include "core/HealthDetector.h"

namespace {

	neon::GameEvent makeDamage(
		std::uint64_t tick,
		int damage
	) {
		neon::GameEvent event{};
		event.tick = tick;
		event.type = neon::GameEventType::DamageApplied;
		event.sourceKind = neon::GameEntityKind::Enemy;
		event.sourceId = 42u;
		event.targetKind = neon::GameEntityKind::Player;
		event.targetId = neon::playerEntityId;
		event.value = damage;
		return event;
	}

}//namespace

int main() {
	neon::HealthDetector detector(
		neon::playerEntityId,
		100
	);

	if (detector.observe(
		1,
		100,
		90,
		{ makeDamage(1, 10) }
	).has_value()) {
		return 1;
	}

	const auto detection =
		detector.observe(
			2,
			90,
			95,
			{ makeDamage(2, 10) }
		);

	if (!detection.has_value() ||
		detection->type !=
		neon::DetectionType::HealthAnomaly ||
		detection->evidence.observedValue !=
		95.0f ||
		detection->evidence.expectedValue !=
		80.0f ||
		detection->confidence != 1.0f) {
		return 2;
	}

	if (detector.observe(
		3,
		90,
		95,
		{ makeDamage(3, 10) }
	).has_value()) {
		return 3;
	}

	if (detector.observe(
		4,
		80,
		80,
		{}
	).has_value()) {
		return 4;
	}

	const auto outOfRange =
		detector.observe(
			5,
			80,
			150,
			{}
		);

	if (!outOfRange.has_value()) {
		return 5;
	}

	return 0;
}