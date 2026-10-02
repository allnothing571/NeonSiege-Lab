#pragma once

#include <array>
#include <cstddef>
#include <random>

#include "core/GameplayConfig.h"
#include "core/RogueliteUpgradeTypes.h"

namespace neon {

	// Standalone first-slice growth module for the roguelite upgrade pass.
	//
	// It owns the core choice, the strengthening levels and the offer rules. It
	// never touches enemies, waves, bosses, maps or the legacy UpgradeSystem,
	// and it computes parameters instead of applying them: no combat behaviour
	// is executed here.
	//
	// Randomness is supplied by the caller. The module never reads a clock and
	// never seeds an engine, so the same engine state plus the same build
	// always produces the same offers.
	class RogueliteUpgradeSystem {
	public:
		static constexpr int offerCapacity = 3;

		void reset();

		// When a same-kind panel is already pending this returns it unchanged
		// instead of drawing again, without touching the build or the engine.
		// A call that cannot legally be honoured (a core was already chosen, or
		// no core exists yet) returns false and leaves any pending panel, the
		// build and the engine exactly as they were.
		// The core panel is a fixed list, so the engine is accepted for
		// interface symmetry with the strengthening panel and left untouched.
		bool generateCoreOffers(
			[[maybe_unused]] std::mt19937& randomEngine);

		// Same rejection and repetition contract as the core panel: the panel
		// is only cleared once a draw is certain to happen, so a refused call
		// cannot destroy pending offers.
		bool generateStrengtheningOffers(
			std::mt19937& randomEngine);

		RogueliteApplyResult applyOffer(int optionIndex);

		int offerCount() const noexcept;
		RogueliteOffer offerAt(int optionIndex) const noexcept;

		RogueliteCoreType selectedCore() const noexcept;
		bool hasCore() const noexcept;

		RogueliteSelectionCounts selectionCounts() const noexcept;

		int level(RogueliteStrengtheningType type) const noexcept;

		RogueliteCoreType lastAppliedCore() const noexcept;
		RogueliteStrengtheningType lastAppliedStrengthening() const noexcept;

		RogueliteStats stats(
			const GameplayConfig& config) const noexcept;

		// Full preview of the build that would result from taking the offer at
		// `optionIndex`. A strengthening offer previews the current core with
		// that card's level temporarily raised; a core offer previews the build
		// that choosing that core would produce with the levels already taken,
		// because the core choice activates its own bonus structure. Nothing is
		// written to the build and no randomness is consumed. Returns false for
		// an invalid index or for an offer that cannot legally be taken, and
		// then leaves `result` untouched.
		bool statsAfter(
			const GameplayConfig& config,
			int optionIndex,
			RogueliteStats& result) const noexcept;

	private:
		RogueliteStats statsForBuild(
			const GameplayConfig& config,
			RogueliteCoreType core,
			const std::array<
				int,
				rogueliteStrengtheningTypeCount
			>& levels
		) const noexcept;

		bool canApply(const RogueliteOffer& offer) const noexcept;

		std::array<
			RogueliteOffer,
			offerCapacity
		> offers_{};

		std::array<
			int,
			rogueliteStrengtheningTypeCount
		> levels_{};

		RogueliteCoreType core_ = RogueliteCoreType::None;
		RogueliteSelectionCounts selectionCounts_{};
		RogueliteCoreType lastAppliedCore_ =
			RogueliteCoreType::None;
		RogueliteStrengtheningType lastAppliedStrengthening_ =
			RogueliteStrengtheningType::None;
		int offerCount_ = 0;
	};

}//namespace neon
