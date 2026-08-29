#include <iostream>
#include "core/Types.h"

int main() {
	neon::Vec2 position = {
	1.0f, 1.0f
	};
	neon::Rect bounds = {
	1.0f, 1.0f ,1.0f, 1.0f
	};
	const neon::Vec2 defaultPosition{};
	const neon::Rect defaultBounds{};

	const bool explicitValuesPassed =
		position.x == 1.0f &&
		position.y == 1.0f &&
		bounds.x == 1.0f &&
		bounds.y == 1.0f &&
		bounds.w == 1.0f &&
		bounds.h == 1.0f;

	const bool defaultValuesPassed =
		defaultPosition.x == 0.0f &&
		defaultPosition.y == 0.0f &&
		defaultBounds.x == 0.0f &&
		defaultBounds.y == 0.0f &&
		defaultBounds.w == 0.0f &&
		defaultBounds.h == 0.0f;

	std::cout << position.x << ' ' << position.y << '\n';
	std::cout << bounds.w << ' ' << bounds.h << '\n';

	const bool passed =
		explicitValuesPassed &&
		defaultValuesPassed;

	return passed ? 0 : 1;
}