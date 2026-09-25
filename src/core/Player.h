#pragma once
#include <cmath>
#include <algorithm>
#include "core/Types.h"
#include "core/EntityId.h"

namespace neon {

	class Player {
	public:
		Player(
			Vec2 position,
			float size,
			float speed,
			int health,
			float invulnerabilityDuration,
			EntityId id = playerEntityId)
			: id_(id),
			position_(position),
			size_(size),
			speed_(speed),
			health_(health),
			invulnerabilityDuration_(
				std::max(0.0f, invulnerabilityDuration)),
			invulnerabilityRemaining_(0.0f)
		{
		}

		void update(
			Vec2 direction,
			float dt,
			const Rect& worldBounds) {
			const float minX = worldBounds.x;
			const float maxX =
				worldBounds.x + worldBounds.w - size_;
			const float minY = worldBounds.y;
			const float maxY =
				worldBounds.y + worldBounds.h - size_;

			if (position_.x <= minX && direction.x < 0.0f) {
				direction.x = 0.0f;
			}
			if (position_.x >= maxX && direction.x > 0.0f) {
				direction.x = 0.0f;
			}
			if (position_.y <= minY && direction.y < 0.0f) {
				direction.y = 0.0f;
			}
			if (position_.y >= maxY && direction.y > 0.0f) {
				direction.y = 0.0f;
			}

			const float directionLength =
				std::sqrt(direction.x * direction.x + direction.y * direction.y);

			if (directionLength > 0.0f) {
				direction.x /= directionLength;
				direction.y /= directionLength;
			}

			updateResolvedMovement(
				Vec2{
					direction.x * speed_ * dt,
					direction.y * speed_ * dt
				},
				dt,
				worldBounds
			);
		}

		void updateResolvedMovement(
			Vec2 displacement,
			float dt,
			const Rect& worldBounds) {

			if (dt > 0.0f) {
				invulnerabilityRemaining_ =
					std::max(
						0.0f,
						invulnerabilityRemaining_ - dt
					);
			}

			const float minX = worldBounds.x;
			const float maxX = worldBounds.x + worldBounds.w - size_;
			const float minY = worldBounds.y;
			const float maxY = worldBounds.y + worldBounds.h - size_;

			position_.x += displacement.x;
			position_.y += displacement.y;

			position_.x = std::clamp(position_.x, minX, maxX);
			position_.y = std::clamp(position_.y, minY, maxY);
		}

		Vec2 center() const {
			return Vec2{ position_.x + size_ / 2.0f, position_.y + size_ / 2.0f };
		}

		EntityId id() const {
			return id_;
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

		Rect movementHitbox() const {
			const float inset = size_ * 0.125f;

			return Rect{
				position_.x + inset,
				position_.y + inset,
				size_ - inset * 2.0f,
				size_ - inset * 2.0f
			};
		}

		bool takeDamage(int damage) {
			if (damage <= 0 ||
				!isAlive() ||
				isInvulnerable()) {
				return false;
			}

			health_ = std::max(0, health_ - damage);
			invulnerabilityRemaining_ =
				invulnerabilityDuration_;

			return true;
		}

		bool isAlive() const {
			return health_ > 0;
		}

		bool isInvulnerable() const {
			return invulnerabilityRemaining_ > 0.0f;
		}

		void reset(Vec2 position, int health) {
			position_ = position;
			health_ = health;
			invulnerabilityRemaining_ = 0.0f;
		}

		int health() const {
			return health_;
		}

	private:
		Vec2 position_;
		float size_;
		float speed_;
		int health_;
		float invulnerabilityDuration_;
		float invulnerabilityRemaining_;
		EntityId id_;
	};

}
