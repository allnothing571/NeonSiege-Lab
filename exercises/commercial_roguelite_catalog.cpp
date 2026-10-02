// Catalog checks for the first roguelite slice.
//
// Verifies that the three cores, six exclusive cards and six general cards are
// all present with unique identifiers, correct categories, correct core
// attribution and correct level caps, and that invalid identifiers are handled
// safely. The catalog is not allowed to reference the legacy UpgradeType
// numbering, which this file checks by never mentioning it and by asserting the
// new counts independently.

#include <cstddef>
#include <cstring>
#include <set>
#include <string>

#include "core/RogueliteUpgradeCatalog.h"
#include "core/RogueliteUpgradeTypes.h"
#include "roguelite_test_support.h"

namespace {

	using namespace neon;

	std::string coreLabel(RogueliteCoreType type) {
		return rogueliteCoreDefinitionFor(type).englishName;
	}

	std::string strengtheningLabel(
		RogueliteStrengtheningType type) {
		return rogueliteStrengtheningDefinitionFor(type)
			.englishName;
	}

	bool textIsUsable(const char* text) {
		return text != nullptr && text[0] != '\0';
	}

	void checkCoreCatalogComposition() {
		CHECK_EQ(
			"core catalog holds exactly 3 selectable cores",
			3u,
			rogueliteSelectableCoreCount);

		int validCores = 0;
		std::set<int> seen;
		for (std::size_t index = 0;
			index < rogueliteCoreTypeCount;
			++index) {
			const auto type =
				static_cast<RogueliteCoreType>(index);
			if (type == RogueliteCoreType::None) {
				CHECK_FALSE(
					"None is not a selectable core",
					rogueliteCoreIsValid(type));
				continue;
			}
			if (type == RogueliteCoreType::Count) {
				CHECK_FALSE(
					"Count is not a selectable core",
					rogueliteCoreIsValid(type));
				continue;
			}

			CHECK_TRUE(
				std::string("core is valid: ") + coreLabel(type),
				rogueliteCoreIsValid(type));
			++validCores;

			const auto& definition =
				rogueliteCoreDefinitionFor(type);
			CHECK_EQ(
				std::string("core identity round trips: ") +
					coreLabel(type),
				static_cast<int>(index),
				static_cast<int>(definition.type));
			CHECK_TRUE(
				std::string("core has a chinese name: ") +
					coreLabel(type),
				textIsUsable(definition.chineseName));
			CHECK_TRUE(
				std::string("core has an english name: ") +
					coreLabel(type),
				textIsUsable(definition.englishName));
			CHECK_TRUE(
				std::string("core has a chinese description: ") +
					coreLabel(type),
				textIsUsable(definition.chineseDescription));
			CHECK_TRUE(
				std::string("core has an english description: ") +
					coreLabel(type),
				textIsUsable(definition.englishDescription));
		}
		CHECK_EQ("valid core count", 3, validCores);
	}

	void checkCoreOfferOrder() {
		const auto& order = rogueliteCoreOfferOrder();
		CHECK_EQ(
			"core offer order length",
			rogueliteSelectableCoreCount,
			order.size());

		std::set<int> seen;
		for (const RogueliteCoreType core : order) {
			CHECK_TRUE(
				std::string("offered core is valid: ") +
					coreLabel(core),
				rogueliteCoreIsValid(core));
			CHECK_TRUE(
				std::string("offered core is not duplicated: ") +
					coreLabel(core),
				seen.insert(static_cast<int>(core))
					.second);
		}
	}

	void checkStrengtheningCatalogComposition() {
		CHECK_EQ(
			"strengthening catalog holds 13 entries "
			"(sentinel plus 12 cards)",
			13u,
			rogueliteStrengtheningTypeCount);

		int validCards = 0;
		int exclusives = 0;
		int offensive = 0;
		int defensive = 0;
		std::set<int> seen;

		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto type =
				static_cast<RogueliteStrengtheningType>(index);
			if (type == RogueliteStrengtheningType::None ||
				type == RogueliteStrengtheningType::Count) {
				CHECK_FALSE(
					std::string("sentinel is not a valid card: ") +
						strengtheningLabel(type),
					rogueliteStrengtheningIsValid(type));
				CHECK_EQ(
					std::string("sentinel has no level cap: ") +
						strengtheningLabel(type),
					0,
					rogueliteMaximumLevel(type));
				continue;
			}

			const std::string label =
				strengtheningLabel(type);
			CHECK_TRUE(
				std::string("card is valid: ") + label,
				rogueliteStrengtheningIsValid(type));
			++validCards;
			CHECK_TRUE(
				std::string("card identifier is unique: ") + label,
				seen.insert(static_cast<int>(type)).second);

			const auto& definition =
				rogueliteStrengtheningDefinitionFor(type);
			CHECK_EQ(
				std::string("card identity round trips: ") + label,
				static_cast<int>(index),
				static_cast<int>(definition.type));
			CHECK_TRUE(
				std::string("card has a chinese name: ") + label,
				textIsUsable(definition.chineseName));
			CHECK_TRUE(
				std::string("card has an english name: ") + label,
				textIsUsable(definition.englishName));
			CHECK_TRUE(
				std::string("card has a chinese description: ") +
					label,
				textIsUsable(definition.chineseDescription));
			CHECK_TRUE(
				std::string("card has an english description: ") +
					label,
				textIsUsable(definition.englishDescription));

			if (definition.exclusive) {
				++exclusives;
				CHECK_EQ(
					std::string("exclusive card level cap is 1: ") +
						label,
					1,
					rogueliteMaximumLevel(type));
				CHECK_EQ(
					std::string("exclusive card category: ") +
						label,
					static_cast<int>(RogueliteCategory::Exclusive),
					static_cast<int>(rogueliteCategoryOf(type)));
				CHECK_TRUE(
					std::string("exclusive card names a core: ") +
						label,
					rogueliteCoreIsValid(
						rogueliteAttributedCoreOf(type)));
			}
			else {
				CHECK_EQ(
					std::string("general card level cap is 2: ") +
						label,
					2,
					rogueliteMaximumLevel(type));
				CHECK_EQ(
					std::string("general card belongs to no core: ") +
						label,
					static_cast<int>(RogueliteCoreType::None),
					static_cast<int>(rogueliteAttributedCoreOf(type)));
				CHECK_FALSE(
					std::string("general card is not exclusive: ") +
						label,
					rogueliteStrengtheningIsExclusive(type));
			}

			switch (rogueliteCategoryOf(type)) {
			case RogueliteCategory::OffensiveSustain:
				++offensive;
				CHECK_FALSE(
					std::string("offensive card is general: ") +
						label,
					definition.exclusive);
				break;
			case RogueliteCategory::DefensiveControl:
				++defensive;
				CHECK_FALSE(
					std::string("defensive card is general: ") +
						label,
					definition.exclusive);
				break;
			case RogueliteCategory::Exclusive:
				CHECK_TRUE(
					std::string("exclusive category is exclusive: ") +
						label,
					definition.exclusive);
				break;
			default:
				CHECK_TRUE(
					std::string("card has a known category: ") +
						label,
					false);
				break;
			}
		}

		CHECK_EQ("valid card count", 12, validCards);
		CHECK_EQ("exclusive card count", 6, exclusives);
		CHECK_EQ("offensive sustain card count", 3, offensive);
		CHECK_EQ("defensive control card count", 3, defensive);
	}

	void checkExclusiveAttribution() {
		// Two exclusive cards per core, and no exclusive card may be offered
		// under a different core.
		int perCore[rogueliteCoreTypeCount]{};
		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto type =
				static_cast<RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsExclusive(type)) {
				continue;
			}
			const RogueliteCoreType owner =
				rogueliteAttributedCoreOf(type);
			++perCore[static_cast<std::size_t>(owner)];
			CHECK_TRUE(
				std::string("exclusive fits its own core: ") +
					strengtheningLabel(type),
				rogueliteStrengtheningFitsCore(type, owner));
		}

		for (const RogueliteCoreType core :
			rogueliteCoreOfferOrder()) {
			CHECK_EQ(
				std::string("exclusive count for ") + coreLabel(core),
				2,
				perCore[static_cast<std::size_t>(core)]);
			int exclusiveForThisCore = 0;
			for (std::size_t index = 0;
				index < rogueliteStrengtheningTypeCount;
				++index) {
				const auto type = static_cast<
					RogueliteStrengtheningType>(index);
				if (!rogueliteStrengtheningIsExclusive(type)) {
					continue;
				}
				if (rogueliteStrengtheningFitsCore(type, core)) {
					++exclusiveForThisCore;
				}
			}
			CHECK_EQ(
				std::string("only own exclusives fit ") +
					coreLabel(core),
				2,
				exclusiveForThisCore);
		}

		// General cards always fit, whatever the core.
		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto type =
				static_cast<RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsValid(type) ||
				rogueliteStrengtheningIsExclusive(type)) {
				continue;
			}
			for (const RogueliteCoreType core :
				rogueliteCoreOfferOrder()) {
				CHECK_TRUE(
					std::string("general card fits ") +
						coreLabel(core) + ": " +
						strengtheningLabel(type),
					rogueliteStrengtheningFitsCore(type, core));
			}
		}
	}

	void checkInvalidIdentifiers() {
		CHECK_FALSE(
			"None core is rejected",
			rogueliteCoreIsValid(RogueliteCoreType::None));
		CHECK_FALSE(
			"Count core is rejected",
			rogueliteCoreIsValid(RogueliteCoreType::Count));
		CHECK_FALSE(
			"None card is rejected",
			rogueliteStrengtheningIsValid(
				RogueliteStrengtheningType::None));
		CHECK_FALSE(
			"Count card is rejected",
			rogueliteStrengtheningIsValid(
				RogueliteStrengtheningType::Count));

		// Deliberately out-of-range values: the lookup must clamp to a safe
		// default rather than index past the array.
		const auto beyond = static_cast<RogueliteCoreType>(999);
		CHECK_FALSE(
			"out-of-range core is rejected",
			rogueliteCoreIsValid(beyond));
		CHECK_EQ(
			"out-of-range core falls back to the default definition",
			0,
			std::strcmp(
				rogueliteCoreDefinitionFor(beyond).englishName,
				rogueliteCoreDefinitionFor(
					RogueliteCoreType::None).englishName));

		const auto beyondCard = static_cast<
			RogueliteStrengtheningType>(999);
		CHECK_FALSE(
			"out-of-range card is rejected",
			rogueliteStrengtheningIsValid(beyondCard));
		CHECK_EQ(
			"out-of-range card falls back to the default definition",
			0,
			std::strcmp(
				rogueliteStrengtheningDefinitionFor(
					beyondCard).englishName,
				rogueliteStrengtheningDefinitionFor(
					RogueliteStrengtheningType::None
				).englishName));
		CHECK_EQ(
			"out-of-range card has no level cap",
			0,
			rogueliteMaximumLevel(beyondCard));
		CHECK_EQ(
			"out-of-range card has no category",
			static_cast<int>(RogueliteCategory::Count),
			static_cast<int>(rogueliteCategoryOf(beyondCard)));
		CHECK_EQ(
			"out-of-range card has no attributed core",
			static_cast<int>(RogueliteCoreType::None),
			static_cast<int>(rogueliteAttributedCoreOf(beyondCard)));
		CHECK_FALSE(
			"out-of-range card never fits a core",
			rogueliteStrengtheningFitsCore(
				beyondCard,
				RogueliteCoreType::FocusFire));
		CHECK_FALSE(
			"valid card never fits an invalid core",
			rogueliteStrengtheningFitsCore(
				RogueliteStrengtheningType::DamageBoost,
				RogueliteCoreType::Count));
	}

	void checkCategoryNamesLanguageIndependent() {
		for (std::size_t index = 0;
			index < rogueliteRealCategoryCount;
			++index) {
			const auto category =
				static_cast<RogueliteCategory>(index);
			CHECK_TRUE(
				std::string("category has an english name: ") +
					std::to_string(index),
				textIsUsable(
					rogueliteCategoryEnglishName(category)));
			CHECK_TRUE(
				std::string("category has a chinese name: ") +
					std::to_string(index),
				textIsUsable(
					rogueliteCategoryChineseName(category)));
		}
		CHECK_EQ(
			"unknown category falls back safely",
			0,
			std::strcmp(
				rogueliteCategoryEnglishName(
					RogueliteCategory::Count),
				"Unknown"));
	}

	void runAll() {
		NEON_TEST_CASE(checkCoreCatalogComposition);
		NEON_TEST_CASE(checkCoreOfferOrder);
		NEON_TEST_CASE(checkStrengtheningCatalogComposition);
		NEON_TEST_CASE(checkExclusiveAttribution);
		NEON_TEST_CASE(checkInvalidIdentifiers);
		NEON_TEST_CASE(checkCategoryNamesLanguageIndependent);
	}

}//namespace

NEON_TEST_MAIN()
