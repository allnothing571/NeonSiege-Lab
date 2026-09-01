#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "core/Obstacle.h"

namespace neon {

	inline bool segmentIntersectsRect(
		Vec2 start,
		Vec2 end,
		const Rect& rect) {

		const Vec2 direction{
			end.x - start.x,
			end.y - start.y
		};

		float tMin = 0.0f;
		float tMax = 1.0f;

		const auto updateInterval =
			[&tMin, &tMax](
				float startValue,
				float directionValue,
				float minimum,
				float maximum) {

					constexpr float epsilon = 0.000001f;

					if (std::fabs(directionValue) <= epsilon) {
						return startValue >= minimum &&
							startValue <= maximum;
					}

					float first =
						(minimum - startValue) / directionValue;

					float second =
						(maximum - startValue) / directionValue;

					if (first > second) {
						std::swap(first, second);
					}

					tMin = std::max(tMin, first);
					tMax = std::min(tMax, second);

					return tMin <= tMax;
			};

		return updateInterval(
			start.x,
			direction.x,
			rect.x,
			rect.x + rect.w
		) &&
			updateInterval(
			start.y,
			direction.y,
			rect.y,
			rect.y + rect.h
		);
	}

	inline bool hasLineOfSight(
		Vec2 observer,
		Vec2 target,
		const std::vector<Obstacle>& obstacles) {

		return std::none_of(
			obstacles.begin(),
			obstacles.end(),
			[observer, target](const Obstacle& obstacle) {
				return segmentIntersectsRect(
					observer,
					target,
					obstacle.bounds()
				);
			}
		);
	}

}//namespace neon