#pragma once
#include "core/Types.h"

namespace neon {
	class Projectile {
	public:
		Projectile(
			Vec2 position,
			Vec2 direction,
			float size,
			float speed)
			: position_(position),
			direction_(direction),
			size_(size),
			speed_(speed) {
		}

		void update(float dt) {
			position_.x += direction_.x * speed_ * dt;
			position_.y += direction_.y * speed_ * dt;
		}

		bool isOutside(const Rect& worldBounds) const {
			return
				position_.x < worldBounds.x - size_ ||
				position_.x > worldBounds.x + worldBounds.w ||
				position_.y < worldBounds.y - size_ ||
				position_.y > worldBounds.y + worldBounds.h;
		}

		Rect bounds() const {
			return Rect{ position_.x, position_.y, size_, size_ };
		}

		void consume() {
			consumed_ = true;
		}

		bool isConsumed() const {
			return consumed_;
		}

	private:
		Vec2 position_;
		Vec2 direction_;
		float size_;
		float speed_;
		bool consumed_ = false;
	};
}