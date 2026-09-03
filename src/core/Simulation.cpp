#include <algorithm>
#include <cmath>
#include <random>

#include "core/Simulation.h"
#include "core/Collision.h"
#include "core/AimSpread.h"
#include "core/LineOfSight.h"

namespace neon {

	Simulation::Simulation(GameplayConfig config)
		: config_(config),
		player_(
			config_.playerStartPosition,
			config_.playerSize,
			config_.playerSpeed,
			config_.playerInitialHealth,
			config_.playerInvulnerabilityDuration,
			playerEntityId
		),
		weapon_(
			config_.magazineCapacity,
			config_.fireInterval,
			config_.reloadDuration
		),
		randomEngine_(config_.randomSeed) {
		reset();
	}

	void Simulation::reset() {
		events_.clear();
		recorderInputs_.clear();
		tick_ = 0;
		nextEntityId_ = playerEntityId + 1;

		randomEngine_.seed(config_.randomSeed);

		player_.reset(
			config_.playerStartPosition,
			config_.playerInitialHealth
		);

		weapon_.reset();

		aimPosition_ = player_.center();

		enemies_.clear();
		projectiles_.clear();

		obstacles_.clear();

		obstacles_.emplace_back(
			Rect{ 220.0f, 180.0f, 240.0f, 32.0f }
		);

		obstacles_.emplace_back(
			Rect{ 620.0f, 320.0f, 180.0f, 40.0f }
		);

		waveManager_.reset();

		const int spawned =
			waveManager_.spawnNextWave(
				enemies_,
				randomEngine_,
				config_,
				player_.bounds(),
				obstacles_,
				nextEntityId_
			);

		if (spawned > 0) {
			GameEvent event{};
			event.type =
				GameEventType::WaveStarted;
			event.value =
				waveManager_.currentWave();

			recordEvent(event);
		}

		state_ = GameState::Playing;
		score_ = 0;
	}

	void Simulation::step(
		const InputCommand& command,
		float fixedDt) {

		if (command.restartPressed &&
			state_ == GameState::Gameover) {
			reset();
			return;
		}

		if (fixedDt <= 0.0f) {
			return;
		}

		++tick_;

		recorderInputs_.push_back(
			ReplayFrame{
				tick_,
				command
			}
		);

		if (command.pausePressed) {
			const GameState previousState =
				state_;

			if (state_ == GameState::Playing) {
				state_ = GameState::Paused;
			}

			else if (state_ == GameState::Paused) {
				state_ = GameState::Playing;
			}

			if (state_ != previousState) {
				GameEvent event{};
				event.type =
					GameEventType::GameStateChanged;
				event.value =
					static_cast<int>(state_);

				recordEvent(event);
			}

			return;
		}

		if (state_ != GameState::Playing) {
			return;
		}

		const bool wasReloading =
			weapon_.isReloading();

		const int ammoBeforeReload =
			weapon_.ammoInMagazine();

		const bool reloadRequested =
			command.reloadPressed &&
			ammoBeforeReload <
			weapon_.magazineCapacity();

		weapon_.update(
			fixedDt,
			command.fireHeld,
			command.reloadPressed
		);

		const bool isReloadingNow =
			weapon_.isReloading();

		if (!wasReloading &&
			reloadRequested) {
			GameEvent event{};
			event.type =
				GameEventType::ReloadStarted;
			event.sourceId =
				player_.id();
			event.sourceKind =
				GameEntityKind::Player;
			event.value =
				ammoBeforeReload;
			event.position =
				player_.center();

			recordEvent(event);
		}

		const bool reloadCompleted =
			(wasReloading &&
				!isReloadingNow) ||
			(reloadRequested &&
				!wasReloading &&
				!isReloadingNow &&
				config_.reloadDuration <= 0.0f);
		if (reloadCompleted) {
			GameEvent event{};
			event.type =
				GameEventType::ReloadCompleted;
			event.sourceId =
				player_.id();
			event.sourceKind =
				GameEntityKind::Player;
			event.value =
				weapon_.ammoInMagazine();
			event.position =
				player_.center();

			recordEvent(event);
		}

		aimPosition_ = command.aimPosition;

		Vec2 allowedMovement =
			command.movement;

		const float probeDistance =
			config_.playerSpeed * fixedDt;

		const auto collidesWithObstacle =
			[this](const Rect& candidate) {
			return std::any_of(
				obstacles_.begin(),
				obstacles_.end(),
				[&candidate](const Obstacle& obstacle) {
					return intersects(
						candidate,
						obstacle.bounds()
					);
				}
			);
			};

		if (allowedMovement.x != 0.0f) {
			Rect horizontalCandidate =
				player_.bounds();

			horizontalCandidate.x +=
				allowedMovement.x > 0.0f
				? probeDistance
				: -probeDistance;

			if (collidesWithObstacle(
				horizontalCandidate)) {

				allowedMovement.x = 0.0f;
			}
		}

		if (allowedMovement.y != 0.0f) {
			Rect verticalCandidate =
				player_.bounds();

			verticalCandidate.y +=
				allowedMovement.y > 0.0f
				? probeDistance
				: -probeDistance;

			if (collidesWithObstacle(
				verticalCandidate)) {

				allowedMovement.y = 0.0f;
			}
		}

		player_.update(
			allowedMovement,
			fixedDt,
			config_.worldBounds
		);

		int activeEnemyProjectileCount =
			static_cast<int>(
				std::count_if(
					projectiles_.begin(),
					projectiles_.end(),
					[](const Projectile& projectile) {
						return !projectile.isConsumed() &&
							projectile.faction() ==
							ProjectileFaction::Enemy;
					}
				)
				);

		for (Enemy& enemy : enemies_) {
			if (!enemy.isAlive()) {
				continue;
			}

			enemy.update(
				player_.center(),
				fixedDt
			);

			if (enemy.kind() == EnemyKind::Shooter) {
				const bool visible =
					hasLineOfSight(
						enemy.center(),
						player_.center(),
						obstacles_
					);

				const bool fireRequested =
					enemy.updateShooterAttack(
						visible,
						fixedDt
					);

				if (fireRequested &&
					activeEnemyProjectileCount <
					config_.maximumEnemyProjectiles) {

					const Vec2 enemyCenter =
						enemy.center();

					const Vec2 playerCenter =
						player_.center();

					Vec2 direction{
						playerCenter.x - enemyCenter.x,
						playerCenter.y - enemyCenter.y
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
							enemyCenter,
							direction,
							config_.enemyProjectileSize,
							config_.enemyProjectileSpeed,
							ProjectileFaction::Enemy,
							config_.enemyProjectileDamage,
							config_.enemyProjectileLifetime,
							nextEntityId_++
						);

						GameEvent event{};
						event.type =
							GameEventType::EnemyShot;
						event.sourceId =
							enemy.id();
						event.sourceKind =
							GameEntityKind::Enemy;
						event.position =
							enemyCenter;

						recordEvent(event);

						++activeEnemyProjectileCount;
					}
				}
			}

			if (intersects(
				player_.hitbox(),
				enemy.hitbox())) {

				const bool damageApplied =
					player_.takeDamage(
						config_.enemyContactDamage
					);

				if (damageApplied) {
					GameEvent damageEvent{};
					damageEvent.type =
						GameEventType::DamageApplied;
					damageEvent.sourceId =
						enemy.id();
					damageEvent.sourceKind =
						GameEntityKind::Enemy;
					damageEvent.targetId =
						player_.id();
					damageEvent.targetKind =
						GameEntityKind::Player;
					damageEvent.value =
						config_.enemyContactDamage;
					damageEvent.position =
						player_.center();

					recordEvent(damageEvent);
				}

				enemy.defeat();

				GameEvent deathEvent{};
				deathEvent.type =
					GameEventType::EntityDied;
				deathEvent.sourceId =
					player_.id();
				deathEvent.sourceKind =
					GameEntityKind::Enemy;
				deathEvent.position =
					enemy.center();

				recordEvent(deathEvent);
			}
		}

		for (Projectile& projectile : projectiles_) {
			projectile.update(fixedDt);
		}

		for (Projectile& projectile : projectiles_) {
			if (projectile.isConsumed()) {
				continue;
			}
			const bool hitObstacle =
				std::any_of(
					obstacles_.begin(),
					obstacles_.end(),
					[&projectile](
						const Obstacle& obstacle) {

							return intersects(
								projectile.bounds(),
								obstacle.bounds()
							);
					}
				);

			if (hitObstacle) {
				projectile.consume();
			}
		}

		for (Projectile& projectile : projectiles_) {
			if (projectile.isConsumed() ||
				projectile.faction() !=
				ProjectileFaction::Enemy) {

				continue;
			}

			if (intersects(
				projectile.bounds(),
				player_.hitbox())) {

				const bool wasAlive =
					player_.isAlive();

				GameEvent hitEvent{};
				hitEvent.type =
					GameEventType::ProjectileHit;
				hitEvent.sourceId =
					projectile.id();
				hitEvent.sourceKind =
					GameEntityKind::Projectile;
				hitEvent.targetId =
					player_.id();
				hitEvent.targetKind =
					GameEntityKind::Player;
				hitEvent.value =
					projectile.damage();
				hitEvent.position =
					player_.center();

				projectile.consume();
				recordEvent(hitEvent);

				const bool damageApplied =
					player_.takeDamage(
						projectile.damage()
					);

				if (damageApplied) {
					GameEvent damageEvent{};
					damageEvent.type =
						GameEventType::DamageApplied;
					damageEvent.sourceKind =
						GameEntityKind::Projectile;
					damageEvent.targetKind =
						GameEntityKind::Player;
					damageEvent.value =
						projectile.damage();
					damageEvent.position =
						player_.center();

					recordEvent(damageEvent);
				}

				if (wasAlive &&
					!player_.isAlive()) {

					GameEvent deathEvent{};
					deathEvent.type =
						GameEventType::EntityDied;
					deathEvent.sourceId =
						player_.id();
					deathEvent.sourceKind =
						GameEntityKind::Player;
					deathEvent.position =
						player_.center();

					recordEvent(deathEvent);
				}
			}
		}

		for (Projectile& projectile : projectiles_) {
			if (projectile.isConsumed() ||
				projectile.faction() !=
				ProjectileFaction::Player) {

				continue;
			}

			for (Enemy& enemy : enemies_) {
				if (!enemy.isAlive()) {
					continue;
				}

				if (projectile.faction() !=
					ProjectileFaction::Player) {

					continue;
				}

				if (intersects(
					projectile.bounds(),
					enemy.bounds())) {

					enemy.takeDamage(
						projectile.damage()
					);

					GameEvent hitEvent{};
					hitEvent.type =
						GameEventType::ProjectileHit;
					hitEvent.sourceId =
						projectile.id();
					hitEvent.sourceKind =
						GameEntityKind::Projectile;
					hitEvent.targetId =
						enemy.id();
					hitEvent.targetKind =
						GameEntityKind::Enemy;
					hitEvent.value =
						projectile.damage();
					hitEvent.position =
						enemy.center();

					recordEvent(hitEvent);

					if (projectile.damage() > 0) {
						GameEvent damageEvent{};
						damageEvent.type =
							GameEventType::DamageApplied;
						damageEvent.sourceKind =
							GameEntityKind::Projectile;
						damageEvent.targetKind =
							GameEntityKind::Enemy;
						damageEvent.value =
							projectile.damage();
						damageEvent.position =
							enemy.center();

						recordEvent(damageEvent);
					}

					projectile.consume();
					if (!enemy.isAlive()) {
						score_ +=
							config_.scorePerEnemy;

						GameEvent deathEvent{};
						deathEvent.type =
							GameEventType::EntityDied;
						deathEvent.sourceId =
							enemy.id();
						deathEvent.sourceKind =
							GameEntityKind::Enemy;
						deathEvent.position =
							enemy.center();

						recordEvent(deathEvent);
					}

					break;
				}
			}
		}

		if (!player_.isAlive() &&
			state_ != GameState::Gameover) {

			state_ = GameState::Gameover;

			GameEvent event{};
			event.type =
				GameEventType::GameStateChanged;
			event.value =
				static_cast<int>(state_);

			recordEvent(event);
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

			GameEvent completedEvent;
			completedEvent.type =
				GameEventType::WaveCompleted;
			completedEvent.value =
				waveManager_.currentWave();

			const int spawned =
				waveManager_.spawnNextWave(
					enemies_,
					randomEngine_,
					config_,
					player_.bounds(),
					obstacles_,
					nextEntityId_
				);

			if (spawned > 0) {
				GameEvent startedEvent{};
				startedEvent.type =
					GameEventType::WaveStarted;
				startedEvent.value =
					waveManager_.currentWave();

				recordEvent(startedEvent);
			}
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
			weapon_.fireRequested()) {

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

				if (weapon_.consumeShot()) {
					constexpr float pi =
						3.14159265358979323846f;

					const float maximumSpreadRadians =
						std::max(
							0.0f,
							config_.playerProjectileSpreadDegrees
						) * pi / 180.0f;

					std::uniform_real_distribution<float>
						spreadDistribution(
							-maximumSpreadRadians,
							maximumSpreadRadians
						);

					const float spreadAngle =
						spreadDistribution(randomEngine_);

					direction =
						rotateDirection(
							direction,
							spreadAngle
						);

					projectiles_.emplace_back(
						playerCenter,
						direction,
						config_.playerProjectileSize,
						config_.playerProjectileSpeed,
						ProjectileFaction::Player,
						config_.playerProjectileDamage,
						config_.playerProjectileLifetime,
						nextEntityId_++
					);

					GameEvent event{};
					event.type =
						GameEventType::PlayerShot;
					event.sourceKind =
						GameEntityKind::Player;
					event.sourceId = player_.id();
					event.position =
						playerCenter;

					recordEvent(event);
				}
			}
		}
	}

	GameSnapshot Simulation::snapshot() const {
		GameSnapshot result{};

		result.state = state_;
		result.player.bounds = player_.bounds();
		result.player.health = player_.health();
		result.player.alive = player_.isAlive();

		result.player.invulnerable =
			player_.isInvulnerable();

		result.player.ammoInMagazine =
			weapon_.ammoInMagazine();

		result.player.magazineCapacity =
			weapon_.magazineCapacity();

		result.player.reloading =
			weapon_.isReloading();

		result.player.reloadProgress =
			weapon_.reloadProgress();

		result.aimPosition = aimPosition_;
		result.score = score_;
		result.currentWave = waveManager_.currentWave();

		result.obstacles.reserve(
			obstacles_.size()
		);

		for (const Obstacle& obstacle : obstacles_) {
			result.obstacles.push_back(
				ObstacleSnapshot{
					obstacle.bounds()
				}
			);
		}

		result.enemies.reserve(enemies_.size());

		for (const Enemy& enemy : enemies_) {
			result.enemies.push_back(
				EnemySnapshot{
					enemy.bounds(),
					enemy.kind(),
					enemy.isAlive(),
					enemy.isWarning(),
					enemy.warningProgress()
				}
			);
		}

		result.projectiles.reserve(projectiles_.size());

		for (const Projectile& projectile : projectiles_) {
			result.projectiles.push_back(
				ProjectileSnapshot{
					projectile.bounds(),
					projectile.faction()
				}
			);
		}

		return result;
	}

	GameState Simulation::state() const {
		return state_;
	}

	std::vector<GameEvent> Simulation::consumeEvents() {
		std::vector<GameEvent> result;
		result.swap(events_);
		return result;
	}

	std::vector<ReplayFrame> Simulation::consumeRecordedInputs() {
		std::vector<ReplayFrame> result;
		result.swap(recorderInputs_);
		return result;
	}

	std::uint64_t Simulation::tick() const {
		return tick_;
	}

	void Simulation::recordEvent(
		GameEvent event) {

		event.tick = tick_;
		events_.push_back(event);
	}

}//namespace neon