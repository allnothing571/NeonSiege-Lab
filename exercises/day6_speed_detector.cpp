#include <cmath>

#include "core/SpeedDetector.h"

namespace {

	bool nearlyEqual(
		float first,
		float second
	) {
		return std::fabs(first - second) < 0.00001f;
	}

}//namespace

int main() {
	constexpr float fixedDt =
		1.0f / 60.0f;

	neon::SpeedDetector detector(
		neon::playerEntityId,
		240.0f,
		0.10f,
		3u
	);

	const auto normal =
		detector.observe(
			1,
			{ 100.0f, 100.0f },
			{ 104.0f, 100.0f },
			fixedDt
		);

	if (normal.has_value()) {
		return 1;
	}

	const float diagonalComponent =
		4.0f / std::sqrt(2.0f);

	const auto diagonal =
		detector.observe(
			2,
			{ 104.0f, 100.0f },
			{
				104.0f + diagonalComponent,
				100.0f + diagonalComponent
			},
			fixedDt
		);

	if (diagonal.has_value()) {
		return 2;
	}

	const auto stopped =
		detector.observe(
			3,
			{ 200.0f, 100.0f },
			{ 200.0f, 100.0f },
			fixedDt
		);

	if (stopped.has_value()) {
		return 3;
	}

	if (detector.observe(
		4,
		{ 200.0f, 100.0f },
		{ 208.0f, 100.0f },
		fixedDt
	).has_value()) {
		return 4;
	}

	if (detector.observe(
		5,
		{ 208.0f, 100.0f },
		{ 216.0f, 100.0f },
		fixedDt
	).has_value()) {
		return 5;
	}

	const auto anomaly =
		detector.observe(
			6,
			{ 216.0f, 100.0f },
			{ 224.0f, 100.0f },
			fixedDt
		);

	if (!anomaly.has_value() ||
		anomaly->tick != 6 ||
		anomaly->type !=
		neon::DetectionType::SpeedAnomaly ||
		anomaly->subjectId !=
		neon::playerEntityId ||
		anomaly->evidence.sampleCount != 3 ||
		anomaly->evidence.observedValue <=
		anomaly->evidence.expectedValue ||
		anomaly->confidence <= 0.0f ||
		anomaly->confidence > 1.0f ||
		!nearlyEqual(
			anomaly->evidence.position.x,
			224.0f
		)) {
		return 6;
	}

	const auto sameStreak =
		detector.observe(
			7,
			{ 224.0f, 100.0f },
			{ 232.0f, 100.0f },
			fixedDt
		);

	if (sameStreak.has_value()) {
		return 7;
	}

	const auto normalAfterAnomaly =
		detector.observe(
			8,
			{ 232.0f, 100.0f },
			{ 236.0f, 100.0f },
			fixedDt
		);

	if (normalAfterAnomaly.has_value()) {
		return 8;
	}

	return 0;
}