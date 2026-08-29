#include <iostream>
#include "core/Types.h"

int main() {
	neon::Vec2 position = {
	1.0f, 1.0f
	};
	neon::Rect bounds = {
	1.0f, 1.0f ,1.0f, 1.0f
	};

	std::cout << position.x << ' ' << position.y << '\n';
	std::cout << bounds.w << ' ' << bounds.h << '\n';
}