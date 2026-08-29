#pragma once

#include <cmath>
#include "core/Types.h"

namespace neon {

	class Enemy {
	public:
		Enemy(
			Vec2 position,
			float size,
			float speed,
			int health)
			: position_(position),
			size_(size),
			speed_(speed),
			health_(health) {
		}

		void update(Vec2 target, float dt) {
			Vec2 direction;
			direction.x = target.x - center().x;
			direction.y = target.y - center().y;

			const float directionLength =
				std::sqrt(direction.x * direction.x + direction.y * direction.y);

			if (directionLength > 0.0f) {
				direction.x /= directionLength;
				direction.y /= directionLength;
			}

			position_.x += direction.x * speed_ * dt;
			position_.y += direction.y * speed_ * dt;
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
	};
}
