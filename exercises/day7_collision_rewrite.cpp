#include <iostream>

struct Rect {
	float x;
	float y;
	float w;
	float h;
};

bool overlaps(const Rect& first, const Rect& second) {
	// TODO：独立写出严格 AABB 重叠判断。
	const bool overlapX =
		first.x < second.x + second.w &&
		second.x < first.x + first.w;
	const bool overlapY =
		first.y < second.y + second.h &&
		second.y < first.y + first.h;

	return overlapX && overlapY;
}

int main() {
	const Rect first{ 0.0f, 0.0f, 10.0f, 10.0f };
	const Rect overlap{ 9.0f, 9.0f, 4.0f, 4.0f };
	const Rect separated{ 20.0f, 0.0f, 4.0f, 4.0f };
	const Rect edgeTouch{ 10.0f, 0.0f, 4.0f, 4.0f };

	std::cout << overlaps(first, overlap) << ' '
		<< overlaps(first, separated) << ' '
		<< overlaps(first, edgeTouch) << '\n';
}
