#pragma once

#include <algorithm>

namespace neon {

	class PlayerWeapon {
	public:
		PlayerWeapon(
			int magazineCapacity,
			float fireInterval,
			float reloadDuration)
			:magazineCapacity_(
				std::max(0, magazineCapacity)),
			ammoInMagazine_(magazineCapacity_),
			fireInterval_(
				std::max(0.0f, fireInterval)),
			reloadDuration_(
				std::max(0.0f, reloadDuration)) {
		}

		void reset() {
			ammoInMagazine_ = magazineCapacity_;
			cooldownRemaining_ = 0.0f;
			reloadRemaining_ = 0.0f;
			fireRequested_ = false;
			reloadStartedThisUpdate_ = false;
			reloadCompletedThisUpdate_ = false;
		}

		void update(
			float dt,
			bool fireHeld,
			bool reloadPressed) {

			fireRequested_ = false;
			reloadStartedThisUpdate_ = false;
			reloadCompletedThisUpdate_ = false;

			if (dt <= 0.0f) {
				return;
			}

			if (isReloading()) {
				reloadRemaining_ -= dt;

				if (reloadRemaining_ <= 0.0f) {
					reloadRemaining_ = 0.0f;
					ammoInMagazine_ = magazineCapacity_;
					reloadCompletedThisUpdate_ = true;
				}

				return;
			}

			if (reloadPressed &&
				ammoInMagazine_ < magazineCapacity_) {
				startReload();

				return;
			}

			cooldownRemaining_ =
				std::max(
					0.0f,
					cooldownRemaining_ - dt
				);

			if (fireHeld &&
				ammoInMagazine_ > 0 &&
				cooldownRemaining_ <= 0.0f) {

				fireRequested_ = true;
			}
		}

		bool fireRequested() const {
			return fireRequested_;
		}

		bool consumeShot() {
			if (!fireRequested_ ||
				ammoInMagazine_ <= 0 ||
				reloadRemaining_ > 0.0f) {

				return false;
			}

			--ammoInMagazine_;
			cooldownRemaining_ = fireInterval_;
			fireRequested_ = false;

			if (ammoInMagazine_ == 0) {
				startReload();
			}

			return true;
		}

		int ammoInMagazine() const {
			return ammoInMagazine_;
		}

		int magazineCapacity() const {
			return magazineCapacity_;
		}

		bool isReloading() const {
			return reloadRemaining_ > 0.0f;
		}

		float reloadProgress() const {
			if (!isReloading() ||
				reloadDuration_ <= 0.0f) {

				return 0.0f;
			}

			return std::clamp(
				1.0f -
				reloadRemaining_ / reloadDuration_,
				0.0f,
				1.0f
			);
		}

		bool reloadStartedThisUpdate() const {
			return reloadStartedThisUpdate_;
		}

		bool reloadCompletedThisUpdate() const {
			return reloadCompletedThisUpdate_;
		}

	private:
		void startReload() {
			if (ammoInMagazine_ >= magazineCapacity_ ||
				isReloading()) {

				return;
			}

			reloadStartedThisUpdate_ = true;

			if (reloadDuration_ <= 0.0f) {
				ammoInMagazine_ = magazineCapacity_;
				reloadRemaining_ = 0.0f;
				reloadCompletedThisUpdate_ = true;
			}
			else {
				reloadRemaining_ = reloadDuration_;
			}
		}

		int magazineCapacity_ = 0;
		int ammoInMagazine_ = 0;

		float fireInterval_ = 0.0f;
		float reloadDuration_ = 0.0f;

		float cooldownRemaining_ = 0.0f;
		float reloadRemaining_ = 0.0f;

		bool fireRequested_ = false;
		bool reloadStartedThisUpdate_ = false;
		bool reloadCompletedThisUpdate_ = false;
	};
}
