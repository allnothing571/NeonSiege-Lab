#include <cmath>

#include "core/DetectionEvent.h"

namespace {

	bool nearlyEqual(
		float first,
		float second
	) {
		return std::fabs(first - second) < 0.00001f;
	}

}//namespace

int main() {
	neon::DetectionEvent defaultEvent{};

	const bool defaultValuesCorrect =
		defaultEvent.tick == 0 &&
		defaultEvent.type ==
		neon::DetectionType::None &&
		defaultEvent.subjectId ==
		neon::invalidEntityId &&
		nearlyEqual(
			defaultEvent.confidence,
			0.0f
		) &&
		defaultEvent.evidence.sampleCount == 0;

	neon::DetectionEvent speedEvent{};

	speedEvent.tick = 42;
	speedEvent.type =
		neon::DetectionType::SpeedAnomaly;
	speedEvent.subjectId =
		neon::playerEntityId;
	speedEvent.confidence = 0.85f;

	speedEvent.evidence.observedValue =
		320.0f;
	speedEvent.evidence.expectedValue =
		240.0f;
	speedEvent.evidence.deviation =
		80.0f;
	speedEvent.evidence.sampleCount = 8;
	speedEvent.evidence.position = {
		456.0f,
		246.0f
	};

	const bool assignedValuesCorrect =
		speedEvent.tick == 42 &&
		speedEvent.type ==
		neon::DetectionType::SpeedAnomaly &&
		speedEvent.subjectId ==
		neon::playerEntityId &&
		nearlyEqual(
			speedEvent.confidence,
			0.85f
		) &&
		nearlyEqual(
			speedEvent.evidence.observedValue,
			320.0f
		) &&
		nearlyEqual(
			speedEvent.evidence.expectedValue,
			240.0f
		) &&
		nearlyEqual(
			speedEvent.evidence.deviation,
			80.0f
		) &&
		speedEvent.evidence.sampleCount == 8 &&
		nearlyEqual(
			speedEvent.evidence.position.x,
			456.0f
		) &&
		nearlyEqual(
			speedEvent.evidence.position.y,
			246.0f
		);

	return defaultValuesCorrect &&
		assignedValuesCorrect
		? 0
		: 1;
}