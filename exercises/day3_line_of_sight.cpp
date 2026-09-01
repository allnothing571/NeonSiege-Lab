#include <vector>

#include "core/LineOfSight.h"

int main() {
	std::vector<neon::Obstacle> obstacles;

	obstacles.emplace_back(
		neon::Rect{
			100.0f,
			100.0f,
			200.0f,
			40.0f
		}
	);

	const bool horizontalBlocked =
		!neon::hasLineOfSight(
			neon::Vec2{ 50.0f, 120.0f },
			neon::Vec2{ 350.0f, 120.0f },
			obstacles
		);

	const bool verticalBlocked =
		!neon::hasLineOfSight(
			neon::Vec2{ 150.0f, 50.0f },
			neon::Vec2{ 150.0f, 200.0f },
			obstacles
		);

	const bool clearSight =
		neon::hasLineOfSight(
			neon::Vec2{ 50.0f, 50.0f },
			neon::Vec2{ 350.0f, 50.0f },
			obstacles
		);

	return horizontalBlocked &&
		verticalBlocked &&
		clearSight
		? 0
		: 1;
}