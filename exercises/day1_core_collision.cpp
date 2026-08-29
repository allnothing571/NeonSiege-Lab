#include <iostream>
#include "core/Types.h"
#include "core/Collision.h"

int main() {
	const neon::Rect first{ 0.0f, 0.0f, 10.0f, 10.0f };

	const neon::Rect overlapping{ 5.0f, 5.0f, 10.0f, 10.0f };
	const neon::Rect separated{ 20.0f, 0.0f, 10.0f, 10.0f };
	const neon::Rect edgeTouching{ 10.0f, 0.0f, 10.0f, 10.0f };

	const bool overlapResult =
		neon::intersects(first, overlapping);

	const bool separatedResult =
		neon::intersects(first, separated);

	const bool edgeTouchingResult =
		neon::intersects(first, edgeTouching);

	std::cout
		<< overlapResult << ' '
		<< separatedResult << ' '
		<< edgeTouchingResult << '\n';

	const bool passed =
		overlapResult &&
		!separatedResult &&
		!edgeTouchingResult;

	return passed ? 0 : 1;
}