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

		int ammoInMagazine = 0;
		int magazineCapacity = 0;
		bool reloading = false;
		float reloadProgress = 0.0f;
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

	struct ObstacleSnapshot {
		Rect bounds{};
	};

	struct GameSnapshot
	{
		GameState state = GameState::Playing;

		PlayerSnapshot player{};
		std::vector<EnemySnapshot> enemies{};
		std::vector<ProjectileSnapshot> projectiles{};
		std::vector<ObstacleSnapshot> obstacles;

		Vec2 aimPosition{};

		int score = 0;
		int currentWave = 0;
	};

}//namespace neon