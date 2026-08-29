#include <iostream>
#include "core/Types.h"
#include "core/Collision.h"

int main() {
	const neon::Rect first{ 0.0f, 0.0f, 10.0f, 10.0f };

	const neon::Rect overlapping{ 5.0f, 5.0f, 10.0f, 10.0f };
	const neon::Rect separated{ 20.0f, 0.0f, 10.0f, 10.0f };
	const neon::Rect edgeTouching{ 10.0f, 0.0f, 10.0f, 10.0f };

	std::cout
		<< neon::intersects(first, overlapping) << ' '
		<< neon::intersects(first, separated) << ' '
		<< neon::intersects(first, edgeTouching) << '\n';
}