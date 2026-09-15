#include <cstdint>
#include <vector>

#include "core/AimAssistDetector.h"

namespace {

	neon::GameEvent makePlayerShot(
		std::uint64_t tick
	) {
		neon::GameEvent event{};
		event.tick = tick;
		event.type =
			neon::GameEventType::PlayerShot;
		event.sourceKind =
			neon::GameEntityKind::Player;
		event.sourceId =
			neon::playerEntityId;
		event.position =
		{ 100.0f, 100.0f };
		return event;
	}

	neon::ReplayFrame makeFrame(
		std::uint64_t tick,
		neon::Vec2 aimPosition,
		bool fireHeld
	) {
		neon::ReplayFrame frame{};
		frame.tick = tick;
		frame.command.aimPosition = aimPosition;
		frame.command.fireHeld = fireHeld;
		return frame;
	}

}//namespace

int main() {
	neon::AimAssistDetector detector(
		neon::playerEntityId,
		0.5f,
		3u
	);

	const neon::Vec2 playerCenter{
		100.0f,
		100.0f
	};

	const neon::Vec2 targetCenter{
		200.0f,
		100.0f
	};

	if (detector.observe(
		makeFrame(
			1,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		{ makePlayerShot(1) }
	).has_value()) {

		return 1;
	}

	if (detector.observe(
		makeFrame(
			2,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		{ makePlayerShot(2) }
	).has_value()) {

		return 2;
	}

	const auto detection =
		detector.observe(
			makeFrame(
				3,
				targetCenter,
				true
			),
			playerCenter,
			targetCenter,
			{ makePlayerShot(3) }
		);

	if (!detection.has_value() ||
		detection->type !=
		neon::DetectionType::AimAssist ||
		detection->subjectId !=
		neon::playerEntityId ||
		detection->confidence != 1.0f ||
		detection->evidence.sampleCount != 3u) {

		return 3;
	}

	if (detector.observe(
		makeFrame(
			4,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		{ makePlayerShot(4) }
	).has_value()) {

		return 4;
	}

	if (detector.observe(
		makeFrame(
			5,
			{ 200.0f, 120.0f },
			true
		),
		playerCenter,
		targetCenter,
		{ makePlayerShot(5) }
	).has_value()) {

		return 5;
	}

	if (detector.observe(
		makeFrame(
			6,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		{}
	).has_value()) {

		return 6;
	}

	detector.reset();

	const auto afterReset =
		detector.observe(
			makeFrame(
				7,
				targetCenter,
				true
			),
			playerCenter,
			targetCenter,
			{ makePlayerShot(7) }
		);

	if (afterReset.has_value()) {
		return 7;
	}

	return 0;
}