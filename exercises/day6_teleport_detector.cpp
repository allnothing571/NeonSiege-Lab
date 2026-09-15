#include "core/TeleportDetector.h"

int main() {
	neon::TeleportDetector detector(
		neon::playerEntityId
	);

	const auto normal =
		detector.observe(
			1,
			{ 100.0f, 100.0f },
			{ 104.0f, 100.0f },
			16.0f
		);

	if (normal.has_value()) {
		return 1;
	}

	const auto jump =
		detector.observe(
			2,
			{ 104.0f, 100.0f },
			{ 200.0f, 100.0f },
			16.0f
		);

	if (!jump.has_value() ||
		jump->type !=
		neon::DetectionType::TeleportAnomaly ||
		jump->subjectId !=
		neon::playerEntityId ||
		jump->evidence.sampleCount != 1 ||
		jump->evidence.observedValue <=
		jump->evidence.expectedValue ||
		jump->confidence <= 0.0f ||
		jump->confidence > 1.0f) {
		return 2;
	}

	const auto sameJump =
		detector.observe(
			3,
			{ 200.0f, 100.0f },
			{ 300.0f, 100.0f },
			16.0f
		);

	if (sameJump.has_value()) {
		return 3;
	}

	const auto normalAfterJump =
		detector.observe(
			4,
			{ 300.0f, 100.0f },
			{ 304.0f, 100.0f },
			16.0f
		);

	if (normalAfterJump.has_value()) {
		return 4;
	}

	const auto secondJump =
		detector.observe(
			5,
			{ 304.0f, 100.0f },
			{ 400.0f, 100.0f },
			16.0f
		);

	if (!secondJump.has_value()) {
		return 5;
	}

	const auto invalidSample =
		detector.observe(
			6,
			{ 400.0f, 100.0f },
			{ 450.0f, 100.0f },
			-1.0f
		);

	if (invalidSample.has_value()) {
		return 6;
	}

	return 0;
}