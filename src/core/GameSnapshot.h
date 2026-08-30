#pragma once

#include <vector>

#include "core/GameState.h"
#include "core/Types.h"

namespace neon {

	struct PlayerSnapshot
	{
		Rect bounds{};
		int health = 0;
		bool alive = false;
	};

	struct EnemySnapshot
	{
		Rect bounds{};
		bool alive = false;
	};

	struct ProjectileSnapshot
	{
		Rect bounds{};
	};

	struct GameSnapshot
	{
		GameState state = GameState::Playing;

		PlayerSnapshot player{};
		std::vector<EnemySnapshot> enemies{};
		std::vector<ProjectileSnapshot> projectiles{};

		Vec2 aimPosition{};

		int score = 0;
		int currentWave = 0;
	};

}//namespace neon