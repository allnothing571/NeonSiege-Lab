#pragma once

#include <cstddef>

namespace neon {

	// Core mechanism chosen once per run. Deliberately a different type from
	// the legacy UpgradeType so the old numbering can never be reused here.
	enum class RogueliteCoreType {
		None,
		FocusFire,
		PiercingRounds,
		ArcLink,
		Count
	};

	constexpr std::size_t rogueliteCoreTypeCount =
		static_cast<std::size_t>(RogueliteCoreType::Count);

	// Number of selectable cores. RogueliteCoreType::Count is a sentinel and
	// RogueliteCoreType::None means "no core selected", so the offer order
	// holds exactly this many entries.
	constexpr std::size_t rogueliteSelectableCoreCount = 3;

	// Strengthening cards. Exclusive entries belong to exactly one core,
	// general entries belong to none.
	enum class RogueliteStrengtheningType {
		None,
		DeepFocus,
		LockMomentum,
		DeepPenetration,
		PenetrationAmplifier,
		ExtendedCircuit,
		ConcentratedDischarge,
		DamageBoost,
		FireRate,
		AmmoSystem,
		ArmorCore,
		MobilityCalibration,
		BallisticCalibration,
		Count
	};

	constexpr std::size_t rogueliteStrengtheningTypeCount =
		static_cast<std::size_t>(
			RogueliteStrengtheningType::Count);

	// Catalog grouping. Exclusive means the card is bound to one core,
	// OffensiveSustain covers the output family, DefensiveControl covers the
	// defense and handling family.
	enum class RogueliteCategory {
		Exclusive,
		OffensiveSustain,
		DefensiveControl,
		Count
	};

	constexpr std::size_t rogueliteCategoryCount =
		static_cast<std::size_t>(RogueliteCategory::Count);

	// Number of real catalog categories. RogueliteCategory::Count is a
	// sentinel, so callers that enumerate the categories use this instead.
	constexpr std::size_t rogueliteRealCategoryCount = 3;

	// An option is either a core choice or a strengthening choice. The kind is
	// stored explicitly so nothing has to guess from a name or a value range.
	enum class RogueliteOptionKind {
		None,
		Core,
		Strengthening
	};

	struct RogueliteOffer
	{
		RogueliteOptionKind kind = RogueliteOptionKind::None;
		RogueliteCoreType core = RogueliteCoreType::None;
		RogueliteStrengtheningType strengthening =
			RogueliteStrengtheningType::None;
	};

	// Focus fire: consecutive hits on the same target stack a damage bonus.
	struct FocusFireBonus
	{
		bool enabled = false;
		float damagePerStack = 0.0f;
		int maximumStacks = 0;
		float expirySeconds = 0.0f;
		bool retainsOnKill = false;
		float retentionSeconds = 0.0f;
	};

	// Piercing rounds: the first target takes a bonus, extra targets take a
	// reduced share.
	struct PiercingRoundsBonus
	{
		bool enabled = false;
		float firstTargetMultiplier = 1.0f;
		int maximumExtraTargets = 0;
		float extraTargetMultiplier = 0.0f;
	};

	// Arc link: every authored number of ordinary projectile hits discharges an
	// arc to nearby enemies.
	struct ArcLinkBonus
	{
		bool enabled = false;
		int hitsPerDischarge = 0;
		float primaryTargetMultiplier = 0.0f;
		int maximumChainTargets = 0;
		float chainMultiplier = 0.0f;
		float chainRadiusPixels = 0.0f;
	};

	// Complete preview of every combat parameter derived from the current
	// build. Damage multipliers stay floating point on purpose: rounding is the
	// later combat implementation's job.
	struct RogueliteStats
	{
		float damageMultiplier = 1.0f;
		float fireInterval = 0.0f;
		int magazineCapacity = 0;
		float reloadDuration = 0.0f;
		int maximumHealth = 0;
		float moveSpeed = 0.0f;
		float projectileSpreadDegrees = 0.0f;
		float projectileSpeed = 0.0f;

		FocusFireBonus focusFire{};
		PiercingRoundsBonus piercingRounds{};
		ArcLinkBonus arcLink{};
	};

	// Result of applying an offer. ArmorCore is the only strengthening whose
	// selection has a one-shot side effect, so it is reported instead of being
	// applied to Player from the core layer.
	struct RogueliteApplyResult
	{
		bool accepted = false;
		int restoredHealth = 0;
	};

	// Running tally of what the player has taken this run.
	struct RogueliteSelectionCounts
	{
		int coreSelections = 0;
		int strengtheningSelections = 0;

		int total() const noexcept {
			return coreSelections + strengtheningSelections;
		}
	};

	// Authored tuning values. They live here, next to the parameter shapes, so
	// "current value" is never confused with "base value".
	struct RogueliteTuning
	{
		static constexpr float focusFireDamagePerStack = 0.10f;
		static constexpr int focusFireMaximumStacks = 3;
		static constexpr float focusFireExpirySeconds = 1.5f;

		static constexpr float deepFocusDamagePerStack = 0.15f;
		static constexpr int deepFocusMaximumStacks = 3;
		static constexpr float lockMomentumRetentionSeconds = 1.5f;

		static constexpr float piercingFirstTargetMultiplier = 1.10f;
		static constexpr int piercingMaximumExtraTargets = 1;
		static constexpr float piercingExtraTargetMultiplier = 0.60f;

		static constexpr int deepPenetrationMaximumExtraTargets = 2;
		static constexpr float penetrationAmplifierExtraTargetMultiplier =
			0.85f;

		static constexpr int arcLinkHitsPerDischarge = 3;
		static constexpr float arcLinkPrimaryTargetMultiplier = 0.40f;
		static constexpr int arcLinkMaximumChainTargets = 2;
		static constexpr float arcLinkChainMultiplier = 0.40f;
		static constexpr float arcLinkChainRadiusPixels = 150.0f;

		static constexpr int extendedCircuitMaximumChainTargets = 3;
		static constexpr float concentratedDischargePrimaryTargetMultiplier =
			0.80f;

		static constexpr float damageBoostPerLevel = 0.15f;
		static constexpr float fireRatePerLevel = 0.10f;
		static constexpr int ammoSystemMagazinePerLevel = 3;
		static constexpr float ammoSystemReloadPerLevel = 0.10f;
		static constexpr int armorCoreHealthPerLevel = 15;
		static constexpr int armorCoreRestoredHealth = 15;
		static constexpr float mobilityCalibrationPerLevel = 0.08f;
		static constexpr float ballisticCalibrationSpreadPerLevel = 1.0f;
		static constexpr float ballisticCalibrationSpeedPerLevel = 60.0f;
	};

}//namespace neon
