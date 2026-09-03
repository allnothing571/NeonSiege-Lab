#pragma once

#include "core/Types.h"
#include "core/ProjectileFaction.h"
#include "core/EntityId.h"

namespace neon {
	class Projectile {
	public:
		Projectile(
			Vec2 position,
			Vec2 direction,
			float size,
			float speed,
			ProjectileFaction faction,
			int damage,
			float lifetime,
			EntityId id = invalidEntityId)
			: id_(id),
			position_(position),
			direction_(direction),
			size_(size),
			speed_(speed),
			faction_(faction),
			damage_(
				damage > 0
				? damage
				: 0),
			remainingLifetime_(lifetime),
			consumed_(lifetime <= 0.0f) {
		}

		void update(float dt) {
			if (consumed_ || dt <= 0.0f) {
				return;
			}

			position_.x += direction_.x * speed_ * dt;
			position_.y += direction_.y * speed_ * dt;

			remainingLifetime_ -= dt;

			if (remainingLifetime_ <= 0.0f) {
				consumed_ = true;
			}
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

		ProjectileFaction faction() const {
			return faction_;
		}

		int damage() const {
			return damage_;
		}

		EntityId id() const {
			return id_;
		}

	private:
		Vec2 position_;
		Vec2 direction_;
		float size_;
		float speed_;
		ProjectileFaction faction_;
		int damage_;
		float remainingLifetime_;
		bool consumed_ = false;
		EntityId id_;
	};
}