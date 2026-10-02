#pragma once

#include <array>

#include "core/RogueliteUpgradeTypes.h"

namespace neon {

	// One strengthening card description. `attributedCore` is meaningful only
	// when `exclusive` is true; general entries belong to no core.
	struct RogueliteStrengtheningDefinition
	{
		RogueliteStrengtheningType type =
			RogueliteStrengtheningType::None;
		RogueliteCategory category =
			RogueliteCategory::Exclusive;
		bool exclusive = false;
		RogueliteCoreType attributedCore =
			RogueliteCoreType::None;
		int maximumLevel = 0;
		const char* chineseName = "";
		const char* englishName = "";
		const char* chineseDescription = "";
		const char* englishDescription = "";
	};

	// One core description. The tuning itself is not stored per core: the
	// authored numbers live in RogueliteTuning and are resolved by
	// RogueliteUpgradeSystem so the system stays the single place that turns a
	// build into parameters.
	struct RogueliteCoreDefinition
	{
		RogueliteCoreType type = RogueliteCoreType::None;
		const char* chineseName = "";
		const char* englishName = "";
		const char* chineseDescription = "";
		const char* englishDescription = "";
	};

	// Immutable catalog access. Every lookup is total: an out-of-range or
	// sentinel identifier returns a safe default instead of reading past the
	// array.
	const std::array<
		RogueliteCoreDefinition,
		rogueliteCoreTypeCount
	>& rogueliteCoreDefinitions() noexcept;

	const std::array<
		RogueliteStrengtheningDefinition,
		rogueliteStrengtheningTypeCount
	>& rogueliteStrengtheningDefinitions() noexcept;

	const RogueliteCoreDefinition& rogueliteCoreDefinitionFor(
		RogueliteCoreType type) noexcept;

	const RogueliteStrengtheningDefinition&
		rogueliteStrengtheningDefinitionFor(
			RogueliteStrengtheningType type) noexcept;

	bool rogueliteCoreIsValid(
		RogueliteCoreType type) noexcept;

	bool rogueliteStrengtheningIsValid(
		RogueliteStrengtheningType type) noexcept;

	bool rogueliteStrengtheningIsExclusive(
		RogueliteStrengtheningType type) noexcept;

	RogueliteCategory rogueliteCategoryOf(
		RogueliteStrengtheningType type) noexcept;

	RogueliteCoreType rogueliteAttributedCoreOf(
		RogueliteStrengtheningType type) noexcept;

	int rogueliteMaximumLevel(
		RogueliteStrengtheningType type) noexcept;

	// True when the card may appear for the given core. Exclusive cards require
	// an exact match; general cards are always eligible.
	bool rogueliteStrengtheningFitsCore(
		RogueliteStrengtheningType type,
		RogueliteCoreType core) noexcept;

	// The selectable cores in their fixed presentation order. Core offers never
	// consume randomness, so this order is also the offer order.
	const std::array<
		RogueliteCoreType,
		rogueliteSelectableCoreCount
	>& rogueliteCoreOfferOrder() noexcept;

	// Presentation helpers that do not depend on the display language.
	const char* rogueliteCategoryEnglishName(
		RogueliteCategory category) noexcept;

	const char* rogueliteCategoryChineseName(
		RogueliteCategory category) noexcept;

}//namespace neon
