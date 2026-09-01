#pragma once

#include <cmath>

#include "core/Types.h"

namespace neon {

	inline Vec2 rotateDirection(
		const Vec2& direction,
		float angleRadians) {

		const float cosine =
			std::cos(angleRadians);

		const float sine =
			std::sin(angleRadians);

		return Vec2{
			direction.x * cosine -
			direction.y * sine,

			direction.x * sine +
			direction.y * cosine
		};
	}

}//namespace neon