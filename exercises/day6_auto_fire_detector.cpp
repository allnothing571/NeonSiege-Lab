#include <vector>

#include "core/AutoFireDetector.h"

namespace {

	neon::GameEvent makePlayerShot(
		std::uint64_t tick,
		float x
	) {
		neon::GameEvent event{};
		event.tick = tick;
		event.type =
			neon::GameEventType::PlayerShot;
		event.sourceKind =
			neon::GameEntityKind::Player;
		event.sourceId =
			neon::playerEntityId;
		event.position = { x, 100.0f };
		return event;
	}

}//namespace

int main() {
	neon::AutoFireDetector detector(
		neon::playerEntityId,
		2u
	);

	neon::ReplayFrame heldFrame{};
	heldFrame.tick = 1;
	heldFrame.command.fireHeld = true;

	const std::vector<neon::GameEvent>
		legitimateShotEvents{
			makePlayerShot(1, 120.0f)
	};

	if (detector.observe(
		heldFrame,
		legitimateShotEvents
	).has_value()) {
		return 1;
	}

	neon::ReplayFrame firstUnexpected{};
	firstUnexpected.tick = 2;
	firstUnexpected.command.fireHeld =
		false;

	const std::vector<neon::GameEvent>
		firstUnexpectedEvents{
			makePlayerShot(2, 140.0f)
	};

	if (detector.observe(
		firstUnexpected,
		firstUnexpectedEvents
	).has_value()) {
		return 2;
	}

	neon::ReplayFrame secondUnexpected{};
	secondUnexpected.tick = 3;
	secondUnexpected.command.fireHeld =
		false;

	const std::vector<neon::GameEvent>
		secondUnexpectedEvents{
			makePlayerShot(3, 160.0f)
	};

	const auto detection =
		detector.observe(
			secondUnexpected,
			secondUnexpectedEvents
		);

	if (!detection.has_value() ||
		detection->tick != 3 ||
		detection->type !=
		neon::DetectionType::AutoFire ||
		detection->subjectId !=
		neon::playerEntityId ||
		detection->confidence != 1.0f ||
		detection->evidence.sampleCount != 2 ||
		detection->evidence.observedValue !=
		1.0f ||
		detection->evidence.expectedValue !=
		0.0f) {
		return 3;
	}

	neon::ReplayFrame sameStreak{};
	sameStreak.tick = 4;
	sameStreak.command.fireHeld =
		false;

	const std::vector<neon::GameEvent>
		sameStreakEvents{
			makePlayerShot(4, 180.0f)
	};

	if (detector.observe(
		sameStreak,
		sameStreakEvents
	).has_value()) {
		return 4;
	}

	neon::ReplayFrame normalFrame{};
	normalFrame.tick = 5;
	normalFrame.command.fireHeld =
		false;

	if (detector.observe(
		normalFrame,
		{}
	).has_value()) {
		return 5;
	}

	return 0;
}