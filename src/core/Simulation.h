#pragma once

#include <vector>

#include "core/Enemy.h"
#include "core/GameState.h"
#include "core/GameplayConfig.h"
#include "core/InputCommand.h"
#include "core/Player.h"
#include "core/Projectile.h"
#include "core/WaveManager.h"
#include "core/GameSnapshot.h"

namespace neon {

	class Simulation {
	public:
		explicit Simulation(
			GameplayConfig config = GameplayConfig{}
		);

		void reset();

		void step(
			const InputCommand& command,
			float fixedDt
		);

		GameSnapshot snapshot() const;

		GameState state() const;

	private:
		GameplayConfig config_;
		Player player_;
		std::vector<Enemy> enemies_;
		std::vector<Projectile> projectiles_;
		Vec2 aimPosition_{};
		WaveManager waveManager_;

		GameState state_ = GameState::Playing;
		int score_ = 0;

		bool fireWasHeld_ = false;
	};
}