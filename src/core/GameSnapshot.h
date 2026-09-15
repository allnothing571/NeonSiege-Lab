#pragma once

#include <vector>

#include "core/GameState.h"
#include "core/Types.h"
#include "core/EnemyKind.h"
#include "core/ProjectileFaction.h"

namespace neon {

	struct PlayerSnapshot
	{
		Rect bounds{};
		int health = 0;
		int maxHealth = 0;
		bool alive = false;

		bool invulnerable = false;

		int ammoInMagazine = 0;
		int magazineCapacity = 0;
		bool reloading = false;
		float reloadProgress = 0.0f;
	};

	struct EnemySnapshot
	{
		Rect bounds{};
		EnemyKind kind = EnemyKind::Chaser;
		bool alive = false;
		bool warningActive = false;
		float warningProgress = 0.0f;
	};

	struct ProjectileSnapshot
	{
		Rect bounds{};
		ProjectileFaction faction =
			ProjectileFaction::Player;
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
		int maximumWaves = 1;
		float intermissionRemaining = 0.0f;
	};

}//namespace neon
