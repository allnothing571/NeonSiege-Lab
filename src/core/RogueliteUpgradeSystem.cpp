#include "core/RogueliteUpgradeSystem.h"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "core/RogueliteUpgradeCatalog.h"

namespace {

	std::size_t strengtheningIndex(
		neon::RogueliteStrengtheningType type) noexcept {
		return static_cast<std::size_t>(type);
	}

	template <typename Value>
	Value takeRandom(
		std::vector<Value>& values,
		std::mt19937& randomEngine) {
		std::uniform_int_distribution<std::size_t> distribution(
			0,
			values.size() - 1
		);
		const std::size_t index = distribution(randomEngine);
		const Value result = values[index];
		values.erase(values.begin() + index);
		return result;
	}

	bool containsStrengthening(
		const std::array<
			neon::RogueliteOffer,
			neon::RogueliteUpgradeSystem::offerCapacity
		>& offers,
		int offerCount,
		neon::RogueliteStrengtheningType type) noexcept {
		for (int index = 0; index < offerCount; ++index) {
			if (offers[index].kind ==
					neon::RogueliteOptionKind::Strengthening &&
				offers[index].strengthening == type) {
				return true;
			}
		}
		return false;
	}

	// A panel counts as pending only for the kind that would generate it. A
	// leftover core panel must not satisfy a strengthening request, and a
	// leftover strengthening panel must not satisfy a core request.
	bool offersArePending(
		const std::array<
			neon::RogueliteOffer,
			neon::RogueliteUpgradeSystem::offerCapacity
		>& offers,
		int offerCount,
		neon::RogueliteOptionKind kind) noexcept {
		return offerCount > 0 &&
			offers[0].kind == kind;
	}

}//namespace

namespace neon {

	void RogueliteUpgradeSystem::reset() {
		offers_.fill(RogueliteOffer{});
		levels_.fill(0);
		core_ = RogueliteCoreType::None;
		selectionCounts_ = RogueliteSelectionCounts{};
		lastAppliedCore_ = RogueliteCoreType::None;
		lastAppliedStrengthening_ =
			RogueliteStrengtheningType::None;
		offerCount_ = 0;
	}

	bool RogueliteUpgradeSystem::generateCoreOffers(
		[[maybe_unused]] std::mt19937& randomEngine) {
		// Same-kind repetition returns the pending panel untouched: generating
		// again must never silently redraw, and it must not consume randomness.
		if (offersArePending(
			offers_,
			offerCount_,
			RogueliteOptionKind::Core)) {
			return true;
		}

		// Every rejection is decided before any state is touched, so a refused
		// call leaves a pending strengthening panel exactly as it was.
		if (hasCore()) {
			return false;
		}

		offers_.fill(RogueliteOffer{});
		offerCount_ = 0;

		const auto& order = rogueliteCoreOfferOrder();
		for (const RogueliteCoreType core : order) {
			if (!rogueliteCoreIsValid(core) ||
				offerCount_ >= offerCapacity) {
				continue;
			}

			RogueliteOffer offer{};
			offer.kind = RogueliteOptionKind::Core;
			offer.core = core;
			offers_[offerCount_++] = offer;
		}

		return offerCount_ > 0;
	}

	bool RogueliteUpgradeSystem::generateStrengtheningOffers(
		std::mt19937& randomEngine) {
		// Same-kind repetition returns the pending panel untouched: generating
		// again must never silently redraw, and it must not consume randomness.
		if (offersArePending(
			offers_,
			offerCount_,
			RogueliteOptionKind::Strengthening)) {
			return true;
		}

		// Every rejection is decided before any state is touched, so a refused
		// call leaves a pending core panel exactly as it was.
		if (!hasCore()) {
			return false;
		}

		// Only cards already taken and fully levelled general cards are
		// filtered out. A legal card is never hidden for being "not strong
		// enough", and no other core's exclusive card can enter the pool.
		// This is the COMPLETE remaining pool: the number of free slots is
		// deliberately not applied while collecting, because stopping early
		// would bias the draw towards the enum's first entries. Cards already
		// taken into the panel are removed AFTER the guaranteed slots are
		// drawn (see below), because the panel has not been built yet at this
		// point and the guaranteed slots draw from their own containers.
		std::vector<RogueliteStrengtheningType> exclusive;
		std::vector<RogueliteStrengtheningType> offensive;
		std::vector<RogueliteStrengtheningType> defensive;
		std::vector<RogueliteStrengtheningType> general;

		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {

			const auto type =
				static_cast<RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsValid(type)) {
				continue;
			}
			if (level(type) >= rogueliteMaximumLevel(type)) {
				continue;
			}
			if (!rogueliteStrengtheningFitsCore(type, core_)) {
				continue;
			}

			if (rogueliteStrengtheningIsExclusive(type)) {
				exclusive.push_back(type);
				continue;
			}

			general.push_back(type);
			switch (rogueliteCategoryOf(type)) {
			case RogueliteCategory::OffensiveSustain:
				offensive.push_back(type);
				break;
			case RogueliteCategory::DefensiveControl:
				defensive.push_back(type);
				break;
			default:
				break;
			}
		}

		// The panel is cleared only once the draw is certain to happen.
		offers_.fill(RogueliteOffer{});
		offerCount_ = 0;

		auto placeCandidate = [this](
			RogueliteStrengtheningType type) {
			RogueliteOffer offer{};
			offer.kind =
				RogueliteOptionKind::Strengthening;
			offer.strengthening = type;
			offers_[offerCount_++] = offer;
		};

		// Slot A: this core's exclusive card, when one is still available.
		// Determined first so the exclusive guarantee survives the later
		// filtering. An exclusive never occupies more than this one slot.
		if (!exclusive.empty()) {
			placeCandidate(takeRandom(exclusive, randomEngine));
		}

		// Slot B then slot C: one offensive-sustain card, then one
		// defensive-control card.
		if (!offensive.empty()) {
			placeCandidate(takeRandom(offensive, randomEngine));
		}
		if (!defensive.empty()) {
			placeCandidate(takeRandom(defensive, randomEngine));
		}

		// Top up from the full remaining general pool: uniform draws without
		// replacement until the panel is full or the pool runs dry. The
		// guaranteed slots above draw from their own containers, which does
		// NOT remove the card from `general`, so any card that already reached
		// the panel is erased here first. Without this, a top-up draw could
		// repeat an offer that slot B or slot C had just placed.
		general.erase(
			std::remove_if(
				general.begin(),
				general.end(),
				[this](RogueliteStrengtheningType type) {
					return containsStrengthening(
						offers_,
						offerCount_,
						type);
				}
			),
			general.end()
		);

		while (offerCount_ < offerCapacity &&
			!general.empty()) {
			placeCandidate(takeRandom(general, randomEngine));
		}

		if (offerCount_ <= 0) {
			return false;
		}

		// Display order is shuffled only inside the filled slice of the panel,
		// leaving empty slots as safe placeholders.
		std::shuffle(
			offers_.begin(),
			offers_.begin() + offerCount_,
			randomEngine
		);
		return true;
	}

	RogueliteApplyResult RogueliteUpgradeSystem::applyOffer(
		int optionIndex) {
		RogueliteApplyResult result{};
		if (optionIndex < 0 ||
			optionIndex >= offerCount_) {
			return result;
		}

		const RogueliteOffer offer = offers_[optionIndex];
		if (!canApply(offer)) {
			return result;
		}

		if (offer.kind == RogueliteOptionKind::Core) {
			core_ = offer.core;
			lastAppliedCore_ = offer.core;
			lastAppliedStrengthening_ =
				RogueliteStrengtheningType::None;
			++selectionCounts_.coreSelections;
		}
		else {
			++levels_[strengtheningIndex(
				offer.strengthening)];
			lastAppliedCore_ = RogueliteCoreType::None;
			lastAppliedStrengthening_ =
				offer.strengthening;
			++selectionCounts_.strengtheningSelections;

			if (offer.strengthening ==
				RogueliteStrengtheningType::ArmorCore) {
				result.restoredHealth =
					RogueliteTuning::armorCoreRestoredHealth;
			}
		}

		offers_.fill(RogueliteOffer{});
		offerCount_ = 0;
		result.accepted = true;
		return result;
	}

	int RogueliteUpgradeSystem::offerCount() const noexcept {
		return offerCount_;
	}

	RogueliteOffer RogueliteUpgradeSystem::offerAt(
		int optionIndex) const noexcept {
		if (optionIndex < 0 ||
			optionIndex >= offerCount_) {
			return RogueliteOffer{};
		}
		return offers_[optionIndex];
	}

	RogueliteCoreType
		RogueliteUpgradeSystem::selectedCore() const noexcept {
		return core_;
	}

	bool RogueliteUpgradeSystem::hasCore() const noexcept {
		return rogueliteCoreIsValid(core_);
	}

	RogueliteSelectionCounts
		RogueliteUpgradeSystem::selectionCounts() const noexcept {
		return selectionCounts_;
	}

	int RogueliteUpgradeSystem::level(
		RogueliteStrengtheningType type) const noexcept {
		if (!rogueliteStrengtheningIsValid(type)) {
			return 0;
		}
		return levels_[strengtheningIndex(type)];
	}

	RogueliteCoreType
		RogueliteUpgradeSystem::lastAppliedCore() const noexcept {
		return lastAppliedCore_;
	}

	RogueliteStrengtheningType
		RogueliteUpgradeSystem::lastAppliedStrengthening() const noexcept {
		return lastAppliedStrengthening_;
	}

	RogueliteStats RogueliteUpgradeSystem::stats(
		const GameplayConfig& config) const noexcept {
		return statsForBuild(config, core_, levels_);
	}

	bool RogueliteUpgradeSystem::statsAfter(
		const GameplayConfig& config,
		int optionIndex,
		RogueliteStats& result) const noexcept {
		if (optionIndex < 0 ||
			optionIndex >= offerCount_) {
			return false;
		}

		const RogueliteOffer offer = offers_[optionIndex];
		if (!canApply(offer)) {
			return false;
		}

		// A pending core offer previews the complete build that choosing that
		// core would produce, using the levels already taken. A strengthening
		// offer previews the current core with a temporarily raised level.
		// Neither path writes to the build or touches the caller's engine.
		if (offer.kind == RogueliteOptionKind::Core) {
			result = statsForBuild(config, offer.core, levels_);
			return true;
		}

		auto previewLevels = levels_;
		++previewLevels[strengtheningIndex(
			offer.strengthening)];
		result = statsForBuild(config, core_, previewLevels);
		return true;
	}

	bool RogueliteUpgradeSystem::canApply(
		const RogueliteOffer& offer) const noexcept {
		switch (offer.kind) {
		case RogueliteOptionKind::Core:
			return rogueliteCoreIsValid(offer.core) &&
				!hasCore();
		case RogueliteOptionKind::Strengthening:
			if (!hasCore()) {
				return false;
			}
			if (!rogueliteStrengtheningIsValid(
				offer.strengthening)) {
				return false;
			}
			if (!rogueliteStrengtheningFitsCore(
				offer.strengthening,
				core_)) {
				return false;
			}
			return level(offer.strengthening) <
				rogueliteMaximumLevel(
					offer.strengthening);
		default:
			return false;
		}
	}

	RogueliteStats RogueliteUpgradeSystem::statsForBuild(
		const GameplayConfig& config,
		RogueliteCoreType core,
		const std::array<
			int,
			rogueliteStrengtheningTypeCount
		>& levels
	) const noexcept {
		const int damageBoostLevel = levels[strengtheningIndex(
			RogueliteStrengtheningType::DamageBoost)];
		const int fireRateLevel = levels[strengtheningIndex(
			RogueliteStrengtheningType::FireRate)];
		const int ammoSystemLevel = levels[strengtheningIndex(
			RogueliteStrengtheningType::AmmoSystem)];
		const int armorCoreLevel = levels[strengtheningIndex(
			RogueliteStrengtheningType::ArmorCore)];
		const int mobilityLevel = levels[strengtheningIndex(
			RogueliteStrengtheningType::MobilityCalibration)];
		const int ballisticLevel = levels[strengtheningIndex(
			RogueliteStrengtheningType::BallisticCalibration)];

		RogueliteStats result{};
		result.damageMultiplier =
			1.0f + RogueliteTuning::damageBoostPerLevel *
				static_cast<float>(damageBoostLevel);
		result.fireInterval = std::max(
			0.0f,
			config.fireInterval /
				(1.0f +
					RogueliteTuning::fireRatePerLevel *
					static_cast<float>(fireRateLevel)));
		result.magazineCapacity = std::max(
			0,
			config.magazineCapacity +
				RogueliteTuning::ammoSystemMagazinePerLevel *
				ammoSystemLevel);
		result.reloadDuration = std::max(
			0.0f,
			config.reloadDuration -
				RogueliteTuning::ammoSystemReloadPerLevel *
				static_cast<float>(ammoSystemLevel));
		result.maximumHealth = std::max(
			0,
			config.playerInitialHealth +
				RogueliteTuning::armorCoreHealthPerLevel *
				armorCoreLevel);
		result.moveSpeed = std::max(
			0.0f,
			config.playerSpeed *
				(1.0f +
					RogueliteTuning::mobilityCalibrationPerLevel *
					static_cast<float>(mobilityLevel)));
		result.projectileSpreadDegrees = std::max(
			0.0f,
			config.playerProjectileSpreadDegrees -
				RogueliteTuning::ballisticCalibrationSpreadPerLevel *
				static_cast<float>(ballisticLevel));
		result.projectileSpeed = std::max(
			0.0f,
			config.playerProjectileSpeed +
				RogueliteTuning::ballisticCalibrationSpeedPerLevel *
				static_cast<float>(ballisticLevel));

		result.focusFire = FocusFireBonus{};
		result.piercingRounds = PiercingRoundsBonus{};
		result.arcLink = ArcLinkBonus{};

		if (rogueliteCoreIsValid(core)) {
			const int deepFocusLevel = levels[strengtheningIndex(
				RogueliteStrengtheningType::DeepFocus)];
			const int lockMomentumLevel = levels[strengtheningIndex(
				RogueliteStrengtheningType::LockMomentum)];
			const int deepPenetrationLevel = levels[strengtheningIndex(
				RogueliteStrengtheningType::DeepPenetration)];
			const int amplifierLevel = levels[strengtheningIndex(
				RogueliteStrengtheningType::PenetrationAmplifier)];
			const int extendedCircuitLevel = levels[strengtheningIndex(
				RogueliteStrengtheningType::ExtendedCircuit)];
			const int concentratedLevel = levels[strengtheningIndex(
				RogueliteStrengtheningType::ConcentratedDischarge)];

			switch (core) {
			case RogueliteCoreType::FocusFire: {
				result.focusFire.enabled = true;
				result.focusFire.damagePerStack =
					deepFocusLevel > 0
					? RogueliteTuning::deepFocusDamagePerStack
					: RogueliteTuning::focusFireDamagePerStack;
				result.focusFire.maximumStacks =
					deepFocusLevel > 0
					? RogueliteTuning::deepFocusMaximumStacks
					: RogueliteTuning::focusFireMaximumStacks;
				result.focusFire.expirySeconds =
					RogueliteTuning::focusFireExpirySeconds;
				result.focusFire.retainsOnKill =
					lockMomentumLevel > 0;
				result.focusFire.retentionSeconds =
					lockMomentumLevel > 0
					? RogueliteTuning::lockMomentumRetentionSeconds
					: 0.0f;
				break;
			}
			case RogueliteCoreType::PiercingRounds: {
				result.piercingRounds.enabled = true;
				result.piercingRounds.firstTargetMultiplier =
					RogueliteTuning::piercingFirstTargetMultiplier;
				result.piercingRounds.maximumExtraTargets =
					deepPenetrationLevel > 0
					? RogueliteTuning::
						deepPenetrationMaximumExtraTargets
					: RogueliteTuning::
						piercingMaximumExtraTargets;
				result.piercingRounds.extraTargetMultiplier =
					amplifierLevel > 0
					? RogueliteTuning::
						penetrationAmplifierExtraTargetMultiplier
					: RogueliteTuning::
						piercingExtraTargetMultiplier;
				break;
			}
			case RogueliteCoreType::ArcLink: {
				result.arcLink.enabled = true;
				result.arcLink.hitsPerDischarge =
					RogueliteTuning::
						arcLinkHitsPerDischarge;
				result.arcLink.primaryTargetMultiplier =
					concentratedLevel > 0
					? RogueliteTuning::
						concentratedDischargePrimaryTargetMultiplier
					: RogueliteTuning::
						arcLinkPrimaryTargetMultiplier;
				result.arcLink.maximumChainTargets =
					extendedCircuitLevel > 0
					? RogueliteTuning::
						extendedCircuitMaximumChainTargets
					: RogueliteTuning::
						arcLinkMaximumChainTargets;
				result.arcLink.chainMultiplier =
					RogueliteTuning::arcLinkChainMultiplier;
				result.arcLink.chainRadiusPixels =
					RogueliteTuning::arcLinkChainRadiusPixels;
				break;
			}
			default:
				break;
			}
		}

		return result;
	}

}//namespace neon
