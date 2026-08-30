#include <algorithm>
#include <cmath>

#include "core/Simulation.h"
#include "core/Collision.h"

namespace neon {

	Simulation::Simulation(GameplayConfig config)
		: config_(config),
		player_(
			config_.playerStartPosition,
			config_.playerSize,
			config_.playerSpeed,
			config_.playerInitialHealth
		) {
		reset();
	}

	void Simulation::reset() {
		player_.reset(
			config_.playerStartPosition,
			config_.playerInitialHealth
		);

		aimPosition_ = player_.center();

		enemies_.clear();
		projectiles_.clear();

		waveManager_.reset();
		waveManager_.spawnNextWave(enemies_);

		state_ = GameState::Playing;
		score_ = 0;
		fireWasHeld_ = false;
	}

	void Simulation::step(
		const InputCommand& command,
		float fixedDt) {

		const bool firePressed =
			command.fireHeld &&
			!fireWasHeld_;

		fireWasHeld_ = command.fireHeld;

		if (command.restartPressed &&
			state_ == GameState::Gameover) {
			reset();
			return;
		}

		if (command.pausePressed) {
			if (state_ == GameState::Playing) {
				state_ = GameState::Paused;
			}

			else if (state_ == GameState::Paused) {
				state_ = GameState::Playing;
			}

			return;
		}

		if (state_ != GameState::Playing ||
			fixedDt <= 0.0f) {
			return;
		}

		aimPosition_ = command.aimPosition;

		player_.update(
			command.movement,
			fixedDt,
			config_.worldBounds
		);

		for (Enemy& enemy : enemies_) {
			enemy.update(
				player_.center(),
				fixedDt
			);

			if (intersects(
				player_.hitbox(),
				enemy.hitbox())) {

				player_.takeDamage(
					config_.enemyContactDamage
				);

				enemy.defeat();
			}
		}

		if (!player_.isAlive()) {
			state_ = GameState::Gameover;
		}

		for (Projectile& projectile : projectiles_) {
			projectile.update(fixedDt);
		}

		for (Projectile& projectile : projectiles_) {
			if (projectile.isConsumed()) {
				continue;
			}

			for (Enemy& enemy : enemies_) {
				if (!enemy.isAlive()) {
					continue;
				}

				if (intersects(
					projectile.bounds(),
					enemy.bounds())) {

					enemy.takeDamage(
						config_.playerProjectileDamage
					);

					projectile.consume();
					if (!enemy.isAlive()) {
						score_ +=
							config_.scorePerEnemy;
					}

					break;
				}
			}
		}

		enemies_.erase(
			std::remove_if(
				enemies_.begin(),
				enemies_.end(),
				[](const Enemy& enemy) {
					return !enemy.isAlive();
				}
			),
			enemies_.end()
		);

		if (state_ == GameState::Playing &&
			enemies_.empty()) {
			waveManager_.spawnNextWave(enemies_);
		}

		projectiles_.erase(
			std::remove_if(
				projectiles_.begin(),
				projectiles_.end(),
				[this](const Projectile& projectile) {
					return projectile.isConsumed() ||
						projectile.isOutside(
							config_.worldBounds
						);
				}
			),
			projectiles_.end()
		);

		if (state_ == GameState::Playing &&
			firePressed) {

			const Vec2 playerCenter =
				player_.center();

			Vec2 direction{
				aimPosition_.x - playerCenter.x,
				aimPosition_.y - playerCenter.y
			};

			const float directionLength =
				std::sqrt(
					direction.x * direction.x +
					direction.y * direction.y
				);

			if (directionLength > 0.0f) {
				direction.x /= directionLength;
				direction.y /= directionLength;

				projectiles_.emplace_back(
					playerCenter,
					direction,
					config_.playerProjectileSize,
					config_.playerProjectileSpeed
				);
			}
		}
	}

	GameSnapshot Simulation::snapshot() const {
		GameSnapshot result{};

		result.state = state_;
		result.player.bounds = player_.bounds();
		result.player.health = player_.health();
		result.player.alive = player_.isAlive();
		result.aimPosition = aimPosition_;
		result.score = score_;
		result.currentWave = waveManager_.currentWave();

		result.enemies.reserve(enemies_.size());

		for (const Enemy& enemy : enemies_) {
			result.enemies.push_back(
				EnemySnapshot{
					enemy.bounds(),
					enemy.isAlive()
				}
			);
		}

		result.projectiles.reserve(projectiles_.size());

		for (const Projectile& projectile : projectiles_) {
			result.projectiles.push_back(
				ProjectileSnapshot{
					projectile.bounds()
				}
			);
		}

		return result;
	}

	GameState Simulation::state() const {
		return state_;
	}

}//namespace neon