#pragma once

#include "core/Types.h"

namespace neon {

	class Obstacle {
	public:
		explicit Obstacle(Rect bounds)
			: bounds_(bounds) {

		}

		Rect bounds() const {
			return bounds_;
		}

	private:
		Rect bounds_{};
	};

}//namespace neon