#pragma once

#include <cmath>
#include "core/Types.h"
#include "core/EnemyKind.h"
#include "core/EntityId.h"

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
		}

		void update(Vec2 target, float dt) {
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

			position_.x += direction.x * speed_ * dt;
			position_.y += direction.y * speed_ * dt;
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
	};
}
