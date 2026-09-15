#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "core/Collision.h"
#include "core/Types.h"
#include "core/EnemyKind.h"
#include "core/EntityId.h"
#include "core/LineOfSight.h"
#include "core/Obstacle.h"

namespace neon {

	class Enemy {
	public:
		Enemy(
			Vec2 position,
			float size,
			float speed,
			int health,
			EnemyKind kind = EnemyKind::Chaser,
			float shooterRetreatDistance = 180.0f,
			float shooterApproachDistance = 320.0f,
			float shooterWarningDuration = 0.3f,
			float shooterFireInterval = 1.2f,
			EntityId id = invalidEntityId)
			: id_(id),
			position_(position),
			size_(size),
			speed_(speed),
			health_(health),
			kind_(kind),
			shooterRetreatDistance_(shooterRetreatDistance),
			shooterApproachDistance_(shooterApproachDistance),
			shooterWarningDuration_(shooterWarningDuration),
			shooterFireInterval_(shooterFireInterval) {
			avoidanceSide_ =
				id_ % 2 == 0 ? 1 : -1;
		}

		void update(
			Vec2 target,
			float dt,
			const Rect& worldBounds) {

			updateInternal(
				target,
				dt,
				worldBounds,
				nullptr
			);
		}

		void update(
			Vec2 target,
			float dt,
			const Rect& worldBounds,
			const std::vector<Obstacle>& obstacles) {

			updateInternal(
				target,
				dt,
				worldBounds,
				&obstacles
			);
		}

		bool updateShooterAttack(
			bool hasLineOfsight,
			float dt) {

			if (kind_ != EnemyKind::Shooter ||
				dt <= 0.0f) {
				return false;
			}

			if (shootCooldownRemaining_ > 0.0f) {
				shootCooldownRemaining_ -= dt;

				if (shootCooldownRemaining_ < 0.0f) {
					shootCooldownRemaining_ = 0.0f;
				}

				warningRemaining_ = 0.0f;
				return false;
			}

			if (!hasLineOfsight) {
				warningRemaining_ = 0.0f;
				return false;
			}

			if (shooterWarningDuration_ <= 0.0f) {
				shootCooldownRemaining_ =
					shooterFireInterval_ > 0.0f
					? shooterFireInterval_
					: 0.0f;

				return true;
			}

			if (warningRemaining_ <= 0.0f) {
				warningRemaining_ =
					shooterWarningDuration_;
			}

			warningRemaining_ -= dt;

			if (warningRemaining_ > 0.0f) {
				return false;
			}

			warningRemaining_ = 0.0f;

			shootCooldownRemaining_ =
				shooterFireInterval_ > 0.0f
				? shooterFireInterval_
				: 0.0f;

			return true;
		}

		bool isWarning() const {
			return warningRemaining_ > 0.0f;
		}

		float warningProgress() const {
			if (!isWarning() ||
				shooterWarningDuration_ <= 0.0f) {

				return 0.0f;
			}

			const float progress =
				1.0f -
				warningRemaining_ /
				shooterWarningDuration_;

			if (progress < 0.0f) {
				return 0.0f;
			}

			if (progress > 1.0f) {
				return 1.0f;
			}

			return progress;
		}

		void takeDamage(int damage) {
			if (damage <= 0) {
				return;
			}

			health_ -= damage;

			if (health_ < 0) {
				health_ = 0;
			}
		}

		bool isAlive() const {
			return health_ > 0;
		}

		EnemyKind kind() const {
			return kind_;
		}

		EntityId id() const {
			return id_;
		}

		void defeat() {
			health_ = 0;
		}

		Vec2 center() const {
			return Vec2{ position_.x + size_ / 2.0f, position_.y + size_ / 2.0f };
		}

		Rect bounds() const {
			return Rect{ position_.x, position_.y, size_, size_ };
		}

		Rect hitbox() const {
			const float inset = size_ * 0.2f;

			return Rect{
				position_.x + inset,
				position_.y + inset,
				size_ - inset * 2.0f,
				size_ - inset * 2.0f
			};
		}

	private:
		static constexpr float movementEpsilon = 0.0001f;
		static constexpr float stuckThreshold = 0.12f;
		static constexpr float navigationClearance = 1.0f;

		void updateInternal(
			Vec2 target,
			float dt,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) {
			Vec2 direction{
			target.x - center().x,
			target.y - center().y
			};

			const float directionLength =
				std::sqrt(direction.x * direction.x + direction.y * direction.y);

			if (directionLength <= 0.0f) {
				return;
			}

			direction.x /= directionLength;
			direction.y /= directionLength;

			if (kind_ == EnemyKind::Shooter) {
				if (directionLength <
					shooterRetreatDistance_) {

					direction.x = -direction.x;
					direction.y = -direction.y;
				}

				else if (directionLength <=
					shooterApproachDistance_) {

					direction = Vec2{
						-direction.y,
						direction.x
					};
				}
			}

			moveWithConstraints(
				direction,
				std::max(0.0f, speed_ * dt),
				dt,
				target,
				worldBounds,
				obstacles
			);
		}

		void moveWithConstraints(
			Vec2 desiredDirection,
			float moveDistance,
			float dt,
			Vec2 target,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) {
			if (moveDistance <= movementEpsilon) {
				return;
			}

			if (obstacles != nullptr && avoidingObstacle_) {
				if (canLeaveAvoidance(
					desiredDirection,
					moveDistance,
					worldBounds,
					obstacles
				)) {
					avoidingObstacle_ = false;
				}
				else if (tryMove(
					Vec2{
						avoidanceDirection_.x * moveDistance,
						avoidanceDirection_.y * moveDistance
					},
					worldBounds,
					obstacles
				)) {
					stuckTime_ = 0.0f;
					return;
				}
				else {
					avoidingObstacle_ = false;
					avoidanceSide_ = -avoidanceSide_;
				}
			}

			const Vec2 delta{
				desiredDirection.x * moveDistance,
				desiredDirection.y * moveDistance
			};

			Rect blockingObstacle{};
			const bool blockedByObstacle =
				findBlockingObstacle(
					delta,
					obstacles,
					blockingObstacle
				);

			if (tryMove(delta, worldBounds, obstacles)) {
				stuckTime_ = 0.0f;
				return;
			}

			if (!blockedByObstacle &&
				tryAxisSlide(delta, worldBounds, obstacles)) {
				stuckTime_ = 0.0f;
				return;
			}

			if (obstacles == nullptr) {
				stuckTime_ = 0.0f;
				return;
			}

			stuckTime_ += std::max(0.0f, dt);

			Vec2 tangentDirection{};
			if (chooseTangentDirection(
				desiredDirection,
				moveDistance,
				target,
				worldBounds,
				obstacles,
				tangentDirection
			) &&
				tryMove(
					Vec2{
						tangentDirection.x * moveDistance,
						tangentDirection.y * moveDistance
					},
					worldBounds,
					obstacles
				)) {
				avoidanceDirection_ = tangentDirection;
				if (blockedByObstacle) {
					avoidanceObstacle_ = expandForNavigation(
						blockingObstacle
					);
					avoidingObstacle_ = true;
				}
				stuckTime_ = 0.0f;
				return;
			}

			if (tryAxisSlide(delta, worldBounds, obstacles)) {
				stuckTime_ = 0.0f;
				return;
			}

			if (stuckTime_ >= stuckThreshold) {
				avoidanceSide_ = -avoidanceSide_;
				stuckTime_ = 0.0f;

				if (chooseTangentDirection(
					desiredDirection,
					moveDistance,
					target,
					worldBounds,
					obstacles,
					tangentDirection
				) &&
					tryMove(
						Vec2{
							tangentDirection.x * moveDistance,
							tangentDirection.y * moveDistance
						},
						worldBounds,
						obstacles
					)) {
					avoidanceDirection_ = tangentDirection;
					if (blockedByObstacle) {
						avoidanceObstacle_ = expandForNavigation(
							blockingObstacle
						);
						avoidingObstacle_ = true;
					}
				}

				if (!avoidingObstacle_) {
					tryEmergencySideStep(
						desiredDirection,
						moveDistance,
						worldBounds,
						obstacles
					);
				}
			}
		}

		Rect expandForNavigation(const Rect& obstacle) const {
			const float expansion =
				size_ / 2.0f + navigationClearance;

			return Rect{
				obstacle.x - expansion,
				obstacle.y - expansion,
				obstacle.w + expansion * 2.0f,
				obstacle.h + expansion * 2.0f
			};
		}

		bool findBlockingObstacle(
			Vec2 delta,
			const std::vector<Obstacle>* obstacles,
			Rect& result) const {
			if (obstacles == nullptr) {
				return false;
			}

			Rect intended = bounds();
			intended.x += delta.x;
			intended.y += delta.y;

			for (const Obstacle& obstacle : *obstacles) {
				const Rect obstacleBounds = obstacle.bounds();

				if (intersects(intended, obstacleBounds)) {
					result = obstacleBounds;
					return true;
				}
			}

			return false;
		}

		bool canLeaveAvoidance(
			Vec2 desiredDirection,
			float moveDistance,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) const {
			Rect nextBounds = bounds();
			nextBounds.x += desiredDirection.x * moveDistance;
			nextBounds.y += desiredDirection.y * moveDistance;

			if (!canOccupy(nextBounds, worldBounds, obstacles)) {
				return false;
			}

			const Vec2 start = center();
			const float probeDistance = std::max(
				size_ * 2.0f,
				moveDistance * 2.0f
			);
			const Vec2 end{
				start.x + desiredDirection.x * probeDistance,
				start.y + desiredDirection.y * probeDistance
			};

			return !segmentIntersectsRect(
				start,
				end,
				avoidanceObstacle_
			);
		}

		bool tryMove(
			Vec2 delta,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) {
			Rect candidate = bounds();
			candidate.x += delta.x;
			candidate.y += delta.y;

			if (!canOccupy(candidate, worldBounds, obstacles)) {
				return false;
			}

			position_.x = candidate.x;
			position_.y = candidate.y;
			return true;
		}

		bool tryAxisSlide(
			Vec2 delta,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) {
			bool moved = false;

			if (delta.x != 0.0f) {
				Rect candidate = bounds();
				candidate.x += delta.x;

				if (canOccupy(candidate, worldBounds, obstacles)) {
					position_.x = candidate.x;
					moved = true;
				}
			}

			if (delta.y != 0.0f) {
				Rect candidate = bounds();
				candidate.y += delta.y;

				if (canOccupy(candidate, worldBounds, obstacles)) {
					position_.y = candidate.y;
					moved = true;
				}
			}

			return moved;
		}

		void addTangentCandidate(
			std::vector<Vec2>& candidates,
			Vec2 candidate) const {
			const float length = std::sqrt(
				candidate.x * candidate.x +
				candidate.y * candidate.y
			);

			if (length <= movementEpsilon) {
				return;
			}

			candidate.x /= length;
			candidate.y /= length;

			for (const Vec2& existing : candidates) {
				if (std::fabs(existing.x - candidate.x) <= movementEpsilon &&
					std::fabs(existing.y - candidate.y) <= movementEpsilon) {
					return;
				}
			}

			candidates.push_back(candidate);
		}

		bool chooseTangentDirection(
			Vec2 desiredDirection,
			float moveDistance,
			Vec2 target,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles,
			Vec2& result) const {
			if (obstacles == nullptr) {
				return false;
			}

			std::vector<Vec2> candidates;
			const float side =
				static_cast<float>(avoidanceSide_);

			addTangentCandidate(
				candidates,
				Vec2{
					-desiredDirection.y * side,
					desiredDirection.x * side
				}
			);
			addTangentCandidate(
				candidates,
				Vec2{
					desiredDirection.y * side,
					-desiredDirection.x * side
				}
			);

			addTangentCandidate(candidates, Vec2{ side, 0.0f });
			addTangentCandidate(candidates, Vec2{ -side, 0.0f });
			addTangentCandidate(candidates, Vec2{ 0.0f, side });
			addTangentCandidate(candidates, Vec2{ 0.0f, -side });

			Rect intended = bounds();
			intended.x += desiredDirection.x * moveDistance;
			intended.y += desiredDirection.y * moveDistance;

			for (const Obstacle& obstacle : *obstacles) {
				const Rect obstacleBounds = obstacle.bounds();

				if (!intersects(intended, obstacleBounds)) {
					continue;
				}

				if (obstacleBounds.w >= obstacleBounds.h) {
					addTangentCandidate(candidates, Vec2{ 1.0f, 0.0f });
					addTangentCandidate(candidates, Vec2{ -1.0f, 0.0f });
				}
				else {
					addTangentCandidate(candidates, Vec2{ 0.0f, 1.0f });
					addTangentCandidate(candidates, Vec2{ 0.0f, -1.0f });
				}
			}

			const Vec2 currentCenter = center();
			const float currentDistance = distanceTo(
				currentCenter,
				target
			);
			float bestScore = -1.0e30f;
			bool found = false;

			for (const Vec2& candidateDirection : candidates) {
				Rect candidate = bounds();
				candidate.x += candidateDirection.x * moveDistance;
				candidate.y += candidateDirection.y * moveDistance;

				if (!canOccupy(candidate, worldBounds, obstacles)) {
					continue;
				}

				const Vec2 nextCenter{
					candidate.x + candidate.w / 2.0f,
					candidate.y + candidate.h / 2.0f
				};
				const float targetProgress =
					currentDistance - distanceTo(nextCenter, target);
				const float alignment =
					candidateDirection.x * desiredDirection.x +
					candidateDirection.y * desiredDirection.y;
				const float score =
					alignment * 2.0f + targetProgress;

				if (!found || score > bestScore) {
					bestScore = score;
					result = candidateDirection;
					found = true;
				}
			}

			return found;
		}

		static float distanceTo(Vec2 first, Vec2 second) {
			const float dx = first.x - second.x;
			const float dy = first.y - second.y;
			return std::sqrt(dx * dx + dy * dy);
		}

		bool tryEmergencySideStep(
			Vec2 desiredDirection,
			float moveDistance,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) {
			if (obstacles == nullptr) {
				return false;
			}

			const float side =
				static_cast<float>(avoidanceSide_);
			const float escapeDistance = std::max(
				movementEpsilon,
				moveDistance * 0.5f
			);
			const Vec2 candidates[] = {
				Vec2{
					-desiredDirection.y * side,
					desiredDirection.x * side
				},
				Vec2{
					desiredDirection.y * side,
					-desiredDirection.x * side
				},
				Vec2{ side, 0.0f },
				Vec2{ 0.0f, side },
				Vec2{ -side, 0.0f },
				Vec2{ 0.0f, -side }
			};

			for (const Vec2& candidate : candidates) {
				const float length = std::sqrt(
					candidate.x * candidate.x +
					candidate.y * candidate.y
				);

				if (length <= movementEpsilon) {
					continue;
				}

				const Vec2 direction{
					candidate.x / length,
					candidate.y / length
				};

				if (tryMove(
					Vec2{
						direction.x * escapeDistance,
						direction.y * escapeDistance
					},
					worldBounds,
					obstacles
				)) {
				avoidanceDirection_ = direction;
				return true;
				}
			}

			return false;
		}

		bool canOccupy(
			const Rect& candidate,
			const Rect& worldBounds,
			const std::vector<Obstacle>* obstacles) const {

			const bool insideWorld =
				candidate.x >= worldBounds.x &&
				candidate.y >= worldBounds.y &&
				candidate.x + candidate.w <=
				worldBounds.x + worldBounds.w &&
				candidate.y + candidate.h <=
				worldBounds.y + worldBounds.h;

			if (!insideWorld || obstacles == nullptr) {
				return insideWorld;
			}

			return std::none_of(
				obstacles->begin(),
				obstacles->end(),
				[&candidate](const Obstacle& obstacle) {
					return intersects(
						candidate,
						obstacle.bounds()
					);
				}
			);
		}

		Vec2 position_;
		float size_;
		float speed_;
		int health_;
		EnemyKind kind_;
		EntityId id_;
		float shooterRetreatDistance_;
		float shooterApproachDistance_;

		float shooterWarningDuration_;
		float shooterFireInterval_;

		float warningRemaining_ = 0.0f;
		float shootCooldownRemaining_ = 0.0f;
		float stuckTime_ = 0.0f;
		bool avoidingObstacle_ = false;
		Rect avoidanceObstacle_{};
		Vec2 avoidanceDirection_{};
		int avoidanceSide_ = 1;
	};
}
