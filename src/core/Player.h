#pragma once
#include <cmath>
#include <algorithm>
#include "core/Types.h"

namespace neon {

	class Player {
	public:
		Player(Vec2 position, float size, float speed, int health)
			: position_(position), size_(size), speed_(speed), health_(health)
		{
		}

		void update(
			Vec2 direction,
			float dt,
			const Rect& worldBounds) {

			const float minX = worldBounds.x;
			const float maxX = worldBounds.x + worldBounds.w - size_;

			const float minY = worldBounds.y;
			const float maxY = worldBounds.y + worldBounds.h - size_;

			if (position_.x <= minX && direction.x < 0) {
				direction.x = 0.0f;
			}
			if (position_.x >= maxX && direction.x > 0) {
				direction.x = 0.0f;
			}
			if (position_.y <= minY && direction.y < 0) {
				direction.y = 0.0f;
			}
			if (position_.y >= maxY && direction.y > 0) {
				direction.y = 0.0f;
			}

			const float directionLength =
				std::sqrt(direction.x * direction.x + direction.y * direction.y);

			if (directionLength > 0.0f) {
				direction.x /= directionLength;
				direction.y /= directionLength;
			}

			position_.x += direction.x * speed_ * dt;
			position_.y += direction.y * speed_ * dt;

			position_.x = std::clamp(position_.x, minX, maxX);
			position_.y = std::clamp(position_.y, minY, maxY);
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

		void reset(Vec2 position, int health) {
			position_ = position;
			health_ = health;
		}

		int health() const {
			return health_;
		}

	private:
		Vec2 position_;
		float size_;
		float speed_;
		int health_;
	};

}