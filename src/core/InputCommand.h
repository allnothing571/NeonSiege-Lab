#pragma once

#include "core/Types.h"

namespace neon {

	struct InputCommand {
		Vec2 movement{};
		Vec2 aimPosition{};

		bool fireHeld = false;
		bool reloadPressed = false;
		bool pausePressed = false;
		bool restartPressed = false;
		int upgradeSelection = -1;
	};
}//namespace neon
