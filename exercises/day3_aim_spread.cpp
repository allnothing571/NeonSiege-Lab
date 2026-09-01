#include <cmath>

#include "core/AimSpread.h"

bool nearlyEqual(float first, float second) {
	constexpr float epsilon = 0.001f;

	return std::fabs(first - second) <= epsilon;
}

int main() {
	constexpr float halfPi =
		1.57079632679f;

	const neon::Vec2 original{
		1.0f,
		0.0f
	};

	const neon::Vec2 unchanged =
		neon::rotateDirection(
			original,
			0.0f
		);

	const bool zeroAnglePassed =
		nearlyEqual(unchanged.x, 1.0f) &&
		nearlyEqual(unchanged.y, 0.0f);

	const neon::Vec2 quarterTurn =
		neon::rotateDirection(
			original,
			halfPi
		);

	const bool quarterTurnPassed =
		nearlyEqual(quarterTurn.x, 0.0f) &&
		nearlyEqual(quarterTurn.y, 1.0f);

	const neon::Vec2 normalized{
		0.6f,
		0.8f
	};

	const neon::Vec2 rotated =
		neon::rotateDirection(
			normalized,
			0.35f
		);

	const float rotatedLengthSquared =
		rotated.x * rotated.x +
		rotated.y * rotated.y;

	const bool lengthPreserved =
		nearlyEqual(
			rotatedLengthSquared,
			1.0f
		);

	return zeroAnglePassed &&
		quarterTurnPassed &&
		lengthPreserved
		? 0
		: 1;
}