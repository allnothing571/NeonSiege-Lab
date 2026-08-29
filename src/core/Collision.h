#pragma once
#include "core/Types.h"

namespace neon {
	inline bool intersects(const Rect& first, const Rect& second) {
		return
			first.x < second.x + second.w &&
			second.x < first.x + first.w &&
			first.y < second.y + second.h &&
			second.y < first.y + first.h;
	}
}