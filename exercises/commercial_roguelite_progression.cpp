// Progression checks for the first roguelite slice: offer generation, core
// selection, level filtering, parameter preview and reset.
//
// The tests drive the public interface only. Nothing here reaches into private
// state, and no test needs a helper that mutates the build directly.

#include <algorithm>
#include <array>
#include <cstddef>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "core/RogueliteUpgradeCatalog.h"
#include "core/RogueliteUpgradeSystem.h"
#include "core/RogueliteUpgradeTypes.h"
#include "roguelite_test_support.h"

namespace {

	using namespace neon;

	constexpr std::uint32_t baseSeed = 20260927u;

	std::string coreLabel(RogueliteCoreType type) {
		return rogueliteCoreDefinitionFor(type).englishName;
	}

	std::string cardLabel(RogueliteStrengtheningType type) {
		return rogueliteStrengtheningDefinitionFor(type)
			.englishName;
	}

	// Picks the requested core, going through the real offer flow.
	bool establishCore(
		RogueliteUpgradeSystem& system,
		RogueliteCoreType core,
		std::mt19937& randomEngine) {
		if (!system.generateCoreOffers(randomEngine)) {
			return false;
		}
		for (int index = 0; index < system.offerCount(); ++index) {
			const RogueliteOffer offer = system.offerAt(index);
			if (offer.kind == RogueliteOptionKind::Core &&
				offer.core == core) {
				return system.applyOffer(index).accepted;
			}
		}
		return false;
	}

	bool cardIsMaxed(
		const RogueliteUpgradeSystem& system,
		RogueliteStrengtheningType card) {
		return system.level(card) >=
			rogueliteMaximumLevel(card);
	}

	// Index of the wanted strengthening in the current panel, or -1.
	int offerIndexFor(
		const RogueliteUpgradeSystem& system,
		RogueliteStrengtheningType card) {
		for (int index = 0; index < system.offerCount(); ++index) {
			const RogueliteOffer offer = system.offerAt(index);
			if (offer.kind ==
					RogueliteOptionKind::Strengthening &&
				offer.strengthening == card) {
				return index;
			}
		}
		return -1;
	}

	// Index of a substitute offer to take when the wanted card is not in the
	// panel: prefer a core while one is still owed, otherwise the first
	// strengthening.
	int chooseAdvanceOffer(
		const RogueliteUpgradeSystem& system) {
		for (int index = 0; index < system.offerCount(); ++index) {
			if (system.offerAt(index).kind ==
				RogueliteOptionKind::Core) {
				return index;
			}
		}
		return 0;
	}

	// Applies offers until the requested card reaches its cap. It never takes
	// a substitute for the target itself, but it does advance the pools when
	// the target is not in the panel, so a card that appears late is still
	// reachable.
	bool takeCard(
		RogueliteUpgradeSystem& system,
		RogueliteStrengtheningType card,
		std::mt19937& randomEngine) {
		for (int attempt = 0; attempt < 256; ++attempt) {
			if (cardIsMaxed(system, card)) {
				return true;
			}
			if (!system.generateStrengtheningOffers(randomEngine)) {
				return false;
			}
			const int offered =
				offerIndexFor(system, card);
			if (offered >= 0) {
				if (!system.applyOffer(offered).accepted) {
					return false;
				}
				continue;
			}
			// The panel is legal but does not contain the wanted card. Take
			// a substitute so the pools advance, then keep looking. The
			// wanted card keeps its place in the pool because it is not fully
			// levelled.
			if (!system.applyOffer(
				chooseAdvanceOffer(system)).accepted) {
				return false;
			}
		}
		return false;
	}

	// Takes exactly one level of a card. It advances the pools when the card
	// is not on the panel, but it stops as soon as one level is applied, so
	// the per-level step can be measured.
	bool takeOneLevel(
		RogueliteUpgradeSystem& system,
		RogueliteStrengtheningType card,
		std::mt19937& randomEngine) {
		for (int attempt = 0; attempt < 256; ++attempt) {
			if (!system.generateStrengtheningOffers(randomEngine)) {
				return false;
			}
			const int offered = offerIndexFor(system, card);
			if (offered >= 0) {
				return system.applyOffer(offered).accepted;
			}
			if (!system.applyOffer(
				chooseAdvanceOffer(system)).accepted) {
				return false;
			}
		}
		return false;
	}

	// Levels every card in the list up to its cap. Each round takes the first
	// listed card that appears in the panel, so progress always happens
	// whenever any wanted card can still be levelled.
	bool levelUpAllCards(
		RogueliteUpgradeSystem& system,
		const std::vector<RogueliteStrengtheningType>& wanted,
		std::mt19937& randomEngine,
		int& roundsUsed) {
		roundsUsed = 0;
		for (int round = 0; round < 200; ++round) {
			bool anyMissing = false;
			for (const auto card : wanted) {
				if (!cardIsMaxed(system, card)) {
					anyMissing = true;
				}
			}
			if (!anyMissing) {
				return true;
			}
			if (!system.generateStrengtheningOffers(randomEngine)) {
				break;
			}
			int chosen = -1;
			for (int index = 0;
				index < system.offerCount() && chosen < 0;
				++index) {
				const RogueliteOffer offer =
					system.offerAt(index);
				if (offer.kind !=
					RogueliteOptionKind::Strengthening) {
					continue;
				}
				for (const auto card : wanted) {
					if (offer.strengthening == card &&
						!cardIsMaxed(system, card)) {
						chosen = index;
						break;
					}
				}
			}
			if (chosen >= 0) {
				if (!system.applyOffer(chosen).accepted) {
					break;
				}
				++roundsUsed;
				continue;
			}
			// Nothing that is being levelled here sits on this panel, so
			// take a card outside the list instead. That keeps the pools
			// moving without spending one of the levels being counted.
			int substitute = -1;
			for (int index = 0; index < system.offerCount(); ++index) {
				const RogueliteOffer offer = system.offerAt(index);
				if (offer.kind !=
					RogueliteOptionKind::Strengthening) {
					continue;
				}
				bool isWanted = false;
				for (const auto card : wanted) {
					if (offer.strengthening == card) {
						isWanted = true;
						break;
					}
				}
				if (!isWanted) {
					substitute = index;
					break;
				}
			}
			if (substitute < 0) {
				break;
			}
			if (!system.applyOffer(substitute).accepted) {
				break;
			}
		}
		bool allMaxed = true;
		for (const auto card : wanted) {
			if (!cardIsMaxed(system, card)) {
				allMaxed = false;
			}
		}
		return allMaxed;
	}

	// Levels a single card up to its cap, taking other cards as needed on the
	// way. Used where exactly one card matters.
	bool levelUpCardFully(
		RogueliteUpgradeSystem& system,
		RogueliteStrengtheningType card,
		std::mt19937& randomEngine) {
		int roundsUsed = 0;
		return levelUpAllCards(
				system,
				std::vector<RogueliteStrengtheningType>{ card },
				randomEngine,
				roundsUsed) &&
			cardIsMaxed(system, card);
	}

	// The six general cards, in catalogue order.
	std::vector<RogueliteStrengtheningType> generalCards() {
		std::vector<RogueliteStrengtheningType> cards;
		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto card = static_cast<
				RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsValid(card) ||
				rogueliteStrengtheningIsExclusive(card)) {
				continue;
			}
			cards.push_back(card);
		}
		return cards;
	}

	// Every card that fits the owned core, exclusives included.
	std::vector<RogueliteStrengtheningType> eligibleCards(
		const RogueliteUpgradeSystem& system) {
		std::vector<RogueliteStrengtheningType> cards;
		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto card = static_cast<
				RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsValid(card)) {
				continue;
			}
			if (!rogueliteStrengtheningFitsCore(
				card,
				system.selectedCore())) {
				continue;
			}
			cards.push_back(card);
		}
		return cards;
	}

	bool offerPanelIsLegal(
		const RogueliteUpgradeSystem& system,
		const std::string& caseName) {
		bool legal = true;
		std::set<int> seen;
		for (int index = 0;
			index < system.offerCount();
			++index) {
			const RogueliteOffer offer = system.offerAt(index);
			if (offer.kind !=
				RogueliteOptionKind::Strengthening) {
				CHECK_TRUE(
					caseName + ": every offer is a strengthening",
					false);
				legal = false;
				continue;
			}
			const auto card = offer.strengthening;
			if (!rogueliteStrengtheningIsValid(card)) {
				CHECK_TRUE(
					caseName + ": no empty placeholder offer",
					false);
				legal = false;
				continue;
			}
			if (!rogueliteStrengtheningFitsCore(
				card,
				system.selectedCore())) {
				CHECK_TRUE(
					caseName + ": offer fits the owned core: " +
						cardLabel(card),
					false);
				legal = false;
			}
			if (!seen.insert(static_cast<int>(card)).second) {
				CHECK_TRUE(
					caseName + ": offer is not duplicated: " +
						cardLabel(card),
					false);
				legal = false;
			}
			if (system.level(card) >=
				rogueliteMaximumLevel(card)) {
				CHECK_TRUE(
					caseName + ": offer is not already maxed: " +
						cardLabel(card),
					false);
				legal = false;
			}
		}
		return legal;
	}

	int countCategory(
		const RogueliteUpgradeSystem& system,
		RogueliteCategory category) {
		int count = 0;
		for (int index = 0; index < system.offerCount(); ++index) {
			const RogueliteOffer offer = system.offerAt(index);
			if (offer.kind ==
					RogueliteOptionKind::Strengthening &&
				rogueliteCategoryOf(offer.strengthening) ==
					category) {
				++count;
			}
		}
		return count;
	}

	bool findOffer(
		const RogueliteUpgradeSystem& system,
		RogueliteStrengtheningType card) {
		for (int index = 0; index < system.offerCount(); ++index) {
			const RogueliteOffer offer = system.offerAt(index);
			if (offer.kind ==
					RogueliteOptionKind::Strengthening &&
				offer.strengthening == card) {
				return true;
			}
		}
		return false;
	}


	// Compare every field, including bonuses belonging to inactive cores.
	void checkStatsEqual(const std::string& label,
		const RogueliteStats& expected, const RogueliteStats& actual) {
#define CHECK_STAT_FLOAT(field) CHECK_NEAR(label + ": " #field, expected.field, actual.field)
#define CHECK_STAT_VALUE(field) CHECK_EQ(label + ": " #field, expected.field, actual.field)
		CHECK_STAT_FLOAT(damageMultiplier);
		CHECK_STAT_FLOAT(fireInterval);
		CHECK_STAT_VALUE(magazineCapacity);
		CHECK_STAT_FLOAT(reloadDuration);
		CHECK_STAT_VALUE(maximumHealth);
		CHECK_STAT_FLOAT(moveSpeed);
		CHECK_STAT_FLOAT(projectileSpreadDegrees);
		CHECK_STAT_FLOAT(projectileSpeed);
		CHECK_STAT_VALUE(focusFire.enabled);
		CHECK_STAT_FLOAT(focusFire.damagePerStack);
		CHECK_STAT_VALUE(focusFire.maximumStacks);
		CHECK_STAT_FLOAT(focusFire.expirySeconds);
		CHECK_STAT_VALUE(focusFire.retainsOnKill);
		CHECK_STAT_FLOAT(focusFire.retentionSeconds);
		CHECK_STAT_VALUE(piercingRounds.enabled);
		CHECK_STAT_FLOAT(piercingRounds.firstTargetMultiplier);
		CHECK_STAT_VALUE(piercingRounds.maximumExtraTargets);
		CHECK_STAT_FLOAT(piercingRounds.extraTargetMultiplier);
		CHECK_STAT_VALUE(arcLink.enabled);
		CHECK_STAT_VALUE(arcLink.hitsPerDischarge);
		CHECK_STAT_FLOAT(arcLink.primaryTargetMultiplier);
		CHECK_STAT_VALUE(arcLink.maximumChainTargets);
		CHECK_STAT_FLOAT(arcLink.chainMultiplier);
		CHECK_STAT_FLOAT(arcLink.chainRadiusPixels);
#undef CHECK_STAT_FLOAT
#undef CHECK_STAT_VALUE
	}

	// Every queryable value of the system, compared field by field.
	void checkStateEqual(const std::string& label,
		const RogueliteUpgradeSystem& expected,
		const RogueliteUpgradeSystem& actual,
		const GameplayConfig& config = GameplayConfig{}) {
		CHECK_EQ(label + ": core", static_cast<int>(expected.selectedCore()),
			static_cast<int>(actual.selectedCore()));
		CHECK_EQ(label + ": has core", expected.hasCore(), actual.hasCore());
		CHECK_EQ(label + ": core count", expected.selectionCounts().coreSelections,
			actual.selectionCounts().coreSelections);
		CHECK_EQ(label + ": strengthening count",
			expected.selectionCounts().strengtheningSelections,
			actual.selectionCounts().strengtheningSelections);
		CHECK_EQ(label + ": total", expected.selectionCounts().total(),
			actual.selectionCounts().total());
		CHECK_EQ(label + ": last core", static_cast<int>(expected.lastAppliedCore()),
			static_cast<int>(actual.lastAppliedCore()));
		CHECK_EQ(label + ": last card",
			static_cast<int>(expected.lastAppliedStrengthening()),
			static_cast<int>(actual.lastAppliedStrengthening()));
		CHECK_EQ(label + ": panel count", expected.offerCount(), actual.offerCount());
		for (int slot = -1; slot <= RogueliteUpgradeSystem::offerCapacity; ++slot) {
			const auto first = expected.offerAt(slot);
			const auto second = actual.offerAt(slot);
			CHECK_EQ(label + ": offer kind", static_cast<int>(first.kind),
				static_cast<int>(second.kind));
			CHECK_EQ(label + ": offer core", static_cast<int>(first.core),
				static_cast<int>(second.core));
			CHECK_EQ(label + ": offer card", static_cast<int>(first.strengthening),
				static_cast<int>(second.strengthening));
		}
		for (std::size_t index = 0; index <= rogueliteStrengtheningTypeCount; ++index) {
			const auto card = static_cast<RogueliteStrengtheningType>(index);
			CHECK_EQ(label + ": level", expected.level(card), actual.level(card));
		}
		checkStatsEqual(label + ": parameters", expected.stats(config),
			actual.stats(config));
	}

	// An independent reference panel: it draws the exclusive guarantee, then
	// one card per general category, and finally samples uniformly from the
	// COMPLETE remaining general pool without replacement. No free-slot limit
	// is applied while building the pool, so the draw cannot favour the enum's
	// earlier entries, and the engine is consumed exactly as often as the real
	// implementation consumes it.
	std::vector<RogueliteStrengtheningType> referencePanel(
		const RogueliteUpgradeSystem& system, std::mt19937& engine) {
		std::vector<RogueliteStrengtheningType> exclusives;
		std::vector<RogueliteStrengtheningType> generals;
		std::vector<RogueliteStrengtheningType> result;
		for (const auto card : eligibleCards(system)) {
			if (cardIsMaxed(system, card)) continue;
			(rogueliteStrengtheningIsExclusive(card) ? exclusives : generals)
				.push_back(card);
		}
		auto draw = [&](std::vector<RogueliteStrengtheningType>& pool) {
			std::uniform_int_distribution<std::size_t> pick(0, pool.size() - 1);
			const std::size_t index = pick(engine);
			result.push_back(pool[index]);
			pool.erase(pool.begin() + index);
		};
		if (!exclusives.empty()) draw(exclusives);
		for (const auto category : { RogueliteCategory::OffensiveSustain,
			RogueliteCategory::DefensiveControl }) {
			std::vector<RogueliteStrengtheningType> pool;
			for (const auto card : generals) {
				if (rogueliteCategoryOf(card) == category) pool.push_back(card);
			}
			if (!pool.empty()) draw(pool);
		}
		generals.erase(std::remove_if(generals.begin(), generals.end(),
			[&](RogueliteStrengtheningType card) {
				return std::find(result.begin(), result.end(), card) != result.end();
			}), generals.end());
		while (result.size() < RogueliteUpgradeSystem::offerCapacity &&
			!generals.empty()) draw(generals);
		std::shuffle(result.begin(), result.end(), engine);
		return result;
	}

	// Offer generation and core selection
	// ---------------------------------------------------------------------

	void checkInitialCoreOffers() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };

		CHECK_EQ(
			"initial core is None",
			static_cast<int>(RogueliteCoreType::None),
			static_cast<int>(system.selectedCore()));
		CHECK_FALSE("initial build has no core", system.hasCore());
		CHECK_EQ("initial offer count is zero", 0, system.offerCount());
		CHECK_EQ(
			"initial selection count is zero",
			0,
			system.selectionCounts().total());

		CHECK_TRUE(
			"core offers can be generated",
			system.generateCoreOffers(randomEngine));
		CHECK_EQ(
			"core offers fill three slots",
			3,
			system.offerCount());

		const auto& order = rogueliteCoreOfferOrder();
		for (int index = 0; index < 3; ++index) {
			const RogueliteOffer offer = system.offerAt(index);
			CHECK_EQ(
				std::string("core offer kind at ") +
					std::to_string(index),
				static_cast<int>(RogueliteOptionKind::Core),
				static_cast<int>(offer.kind));
			CHECK_EQ(
				std::string("core offer order at ") +
					std::to_string(index),
				static_cast<int>(order[static_cast<std::size_t>(
					index)]),
				static_cast<int>(offer.core));
		}
	}

	void checkCoreGenerationDoesNotConsumeRandomness() {
		RogueliteUpgradeSystem first{};
		RogueliteUpgradeSystem second{};
		std::mt19937 firstEngine{ baseSeed };

		const std::mt19937 before = firstEngine;
		CHECK_TRUE(
			"first core panel generated",
			first.generateCoreOffers(firstEngine));
		// Different engines must still yield the same fixed core panel.
		std::mt19937 otherEngine{ baseSeed + 1u };
		CHECK_TRUE(
			"second core panel generated",
			second.generateCoreOffers(otherEngine));

		for (int index = 0; index < 3; ++index) {
			CHECK_EQ(
				std::string("core panel is seed independent at ") +
					std::to_string(index),
				static_cast<int>(first.offerAt(index).core),
				static_cast<int>(second.offerAt(index).core));
		}
		CHECK_TRUE(
			"core generation leaves the caller's engine untouched",
			before == firstEngine);
	}

	void checkStrengtheningNeedsACore() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };

		CHECK_FALSE(
			"strengthening offers are refused before a core exists",
			system.generateStrengtheningOffers(randomEngine));
		CHECK_EQ(
			"refused generation leaves no offers",
			0,
			system.offerCount());
		CHECK_EQ(
			"refused generation leaves no selection",
			0,
			system.selectionCounts().total());
	}

	void checkCoreCanOnlyBeChosenOnce() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };

		CHECK_TRUE(
			"core panel generated",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));
		CHECK_EQ(
			"selected core recorded",
			static_cast<int>(RogueliteCoreType::FocusFire),
			static_cast<int>(system.selectedCore()));
		CHECK_EQ(
			"core selection counted",
			1,
			system.selectionCounts().coreSelections);

		CHECK_FALSE(
			"core panel is refused after a core exists",
			system.generateCoreOffers(randomEngine));
		CHECK_EQ(
			"second core cannot be selected",
			static_cast<int>(RogueliteCoreType::FocusFire),
			static_cast<int>(system.selectedCore()));
		CHECK_EQ(
			"core selection count is still one",
			1,
			system.selectionCounts().coreSelections);
	}

	void checkCorePoolsAreSeparated() {
		for (const RogueliteCoreType core :
			rogueliteCoreOfferOrder()) {
			RogueliteUpgradeSystem system{};
			std::mt19937 randomEngine{ baseSeed };
			CHECK_TRUE(
				std::string("core established: ") + coreLabel(core),
				establishCore(system, core, randomEngine));

			for (int round = 0; round < 8; ++round) {
				if (!system.generateStrengtheningOffers(
					randomEngine)) {
					break;
				}
				const std::string panelName =
					"core pool " + coreLabel(core) +
					" round " + std::to_string(round);
				offerPanelIsLegal(system, panelName);

				for (int index = 0;
					index < system.offerCount();
					++index) {
					const auto card =
						system.offerAt(index).strengthening;
					CHECK_TRUE(
						panelName + ": card fits core: " +
							cardLabel(card),
						rogueliteStrengtheningFitsCore(
							card,
							core));
				}

				if (!system.applyOffer(0).accepted) {
					CHECK_TRUE(
						panelName + ": a legal offer applies",
						false);
					break;
				}
			}
		}
	}

	void checkGeneralCardsAreExcludedWhenMaxed() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for max-level filtering",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));

		CHECK_TRUE(
			"Damage Boost can be maxed",
			levelUpCardFully(
				system,
				RogueliteStrengtheningType::DamageBoost,
				randomEngine));
		CHECK_EQ(
			"Damage Boost reached its cap",
			2,
			system.level(
				RogueliteStrengtheningType::DamageBoost));

		for (int round = 0; round < 12; ++round) {
			if (!system.generateStrengtheningOffers(
				randomEngine)) {
				break;
			}
			CHECK_FALSE(
				"a maxed general card is never offered again",
				findOffer(
					system,
					RogueliteStrengtheningType::DamageBoost));
			if (!system.applyOffer(0).accepted) {
				break;
			}
		}
	}

	void checkExclusiveCardsAreNeverRepeated() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for exclusive filtering",
			establishCore(
				system,
				RogueliteCoreType::ArcLink,
				randomEngine));

		CHECK_TRUE(
			"Extended Circuit can be taken",
			takeCard(
				system,
				RogueliteStrengtheningType::ExtendedCircuit,
				randomEngine));
		CHECK_EQ(
			"Extended Circuit is level one",
			1,
			system.level(
				RogueliteStrengtheningType::ExtendedCircuit));

		for (int round = 0; round < 12; ++round) {
			if (!system.generateStrengtheningOffers(
				randomEngine)) {
				break;
			}
			CHECK_FALSE(
				"an exclusive card is never offered twice",
				findOffer(
					system,
					RogueliteStrengtheningType::ExtendedCircuit));
			if (!system.applyOffer(0).accepted) {
				break;
			}
		}
	}

	void checkNormalPanelGuaranteesEveryCategory() {
		int panelsChecked = 0;
		for (std::uint32_t seed = 0; seed < 25u; ++seed) {
			RogueliteUpgradeSystem system{};
			std::mt19937 randomEngine{ baseSeed + seed };
			if (!establishCore(
					system,
					RogueliteCoreType::PiercingRounds,
					randomEngine)) {
				continue;
			}
			if (!system.generateStrengtheningOffers(randomEngine)) {
				continue;
			}
			if (system.offerCount() != 3) {
				continue;
			}
			++panelsChecked;
			const std::string panelName =
				"full panel seed " + std::to_string(seed);
			CHECK_EQ(
				panelName + ": one exclusive slot",
				1,
				countCategory(
					system,
					RogueliteCategory::Exclusive));
			CHECK_EQ(
				panelName + ": one offensive sustain slot",
				1,
				countCategory(
					system,
					RogueliteCategory::OffensiveSustain));
			CHECK_EQ(
				panelName + ": one defensive control slot",
				1,
				countCategory(
					system,
					RogueliteCategory::DefensiveControl));
			offerPanelIsLegal(system, panelName);
		}
		CHECK_TRUE(
			"at least one full panel was evaluated",
			panelsChecked > 0);
	}

	// ---------------------------------------------------------------------
	// Degraded and exhausted pools
	// ---------------------------------------------------------------------

	void checkExclusiveExhaustionIsHandled() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for exclusive exhaustion",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));

		CHECK_TRUE(
			"Deep Focus taken",
			takeCard(
				system,
				RogueliteStrengtheningType::DeepFocus,
				randomEngine));
		CHECK_TRUE(
			"Lock Momentum taken",
			takeCard(
				system,
				RogueliteStrengtheningType::LockMomentum,
				randomEngine));
		CHECK_EQ(
			"all focus exclusives taken",
			2,
			system.selectionCounts().strengtheningSelections);

		CHECK_TRUE(
			"panel still generates after exclusive exhaustion",
			system.generateStrengtheningOffers(randomEngine));
		CHECK_EQ(
			"degraded panel is filled to capacity",
			3,
			system.offerCount());
		CHECK_EQ(
			"degraded panel has no exclusive slot",
			0,
			countCategory(
				system,
				RogueliteCategory::Exclusive));
		offerPanelIsLegal(
			system,
			"degraded panel after exclusive exhaustion");
	}

	// Levels the six general cards to their caps while never selecting an
	// exclusive. levelUpAllCards() fills its substitute slot with any card
	// outside the list - which can be an exclusive - so it cannot be used to
	// prepare a build that must still own both exclusives.
	bool levelUpAllGenerals(
		RogueliteUpgradeSystem& system,
		std::mt19937& randomEngine,
		int& roundsUsed) {
		// Both iterators must come from one live container: calling
		// generalCards() twice would hand begin() and end() to two different
		// temporaries.
		const auto cards = generalCards();
		const std::set<RogueliteStrengtheningType> wanted(
			cards.begin(), cards.end());
		roundsUsed = 0;
		for (int round = 0; round < 200; ++round) {
			bool anyMissing = false;
			for (const auto card : wanted) {
				if (!cardIsMaxed(system, card)) {
					anyMissing = true;
				}
			}
			if (!anyMissing) {
				return true;
			}
			if (!system.generateStrengtheningOffers(randomEngine)) {
				return false;
			}
			int chosen = -1;
			for (int index = 0;
				index < system.offerCount() && chosen < 0;
				++index) {
				const RogueliteOffer offer = system.offerAt(index);
				if (offer.kind ==
						RogueliteOptionKind::Strengthening &&
					wanted.count(offer.strengthening) > 0) {
					chosen = index;
				}
			}
			if (chosen < 0) {
				return false;
			}
			if (!system.applyOffer(chosen).accepted) {
				return false;
			}
			++roundsUsed;
		}
		return false;
	}

	void checkCategoryExhaustionTopsUpFromRemainingCards() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for category exhaustion",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));

		// Preparation only ever takes general cards, so both FocusFire
		// exclusives must still be unowned when the generals are all maxed.
		int roundsUsed = 0;
		CHECK_TRUE(
			"every general card reaches its cap without touching exclusives",
			levelUpAllGenerals(system, randomEngine, roundsUsed));
		CHECK_EQ(
			"maxing six general cards takes twelve selections",
			12,
			roundsUsed);
		for (const RogueliteStrengtheningType card : generalCards()) {
			CHECK_TRUE(
				std::string("general card maxed: ") + cardLabel(card),
				cardIsMaxed(system, card));
		}
		for (const RogueliteStrengtheningType card : {
			RogueliteStrengtheningType::DeepFocus,
			RogueliteStrengtheningType::LockMomentum
			}) {
			CHECK_TRUE(
				std::string("exclusive still unowned: ") + cardLabel(card),
				rogueliteStrengtheningIsExclusive(card) &&
					system.level(card) == 0);
		}
		CHECK_EQ(
			"only twelve strengthening selections were spent",
			12,
			system.selectionCounts().strengtheningSelections);

		// An exclusive occupies at most ONE slot, so a build with a single
		// remaining exclusive must produce a one-slot panel - not fill the
		// panel with the same category twice.
		CHECK_TRUE(
			"exclusives remain after every general card is maxed",
			system.generateStrengtheningOffers(randomEngine));
		CHECK_EQ(
			"one remaining exclusive occupies one slot",
			1,
			system.offerCount());
		CHECK_EQ(
			"the panel offers exactly one exclusive",
			1,
			countCategory(
				system,
				RogueliteCategory::Exclusive));
		CHECK_EQ(
			"no general card appears once all are maxed",
			0,
			countCategory(
				system,
				RogueliteCategory::OffensiveSustain) +
				countCategory(
					system,
					RogueliteCategory::DefensiveControl));
		offerPanelIsLegal(
			system,
			"panel after every general card is maxed");

		// Taking that exclusive must leave the other one still offerable.
		CHECK_TRUE(
			"degraded panel is selectable",
			system.applyOffer(0).accepted);
		CHECK_TRUE(
			"second exclusive still generates a panel",
			system.generateStrengtheningOffers(randomEngine));
		CHECK_EQ(
			"second exclusive also occupies one slot",
			1,
			system.offerCount());
		CHECK_EQ(
			"second panel offers the other exclusive",
			1,
			countCategory(
				system,
				RogueliteCategory::Exclusive));
		CHECK_TRUE(
			"second exclusive is selectable",
			system.applyOffer(0).accepted);

		// Only after BOTH exclusives are taken is the pool truly exhausted.
		CHECK_FALSE(
			"fully exhausted pool stops offering a panel",
			system.generateStrengtheningOffers(randomEngine));
		CHECK_EQ(
			"exhausted pool leaves no panel",
			0,
			system.offerCount());
	}

	void checkFullExhaustionIsHandled() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for full exhaustion",
			establishCore(
				system,
				RogueliteCoreType::ArcLink,
				randomEngine));

		// ArcLink offers eight buildable cards: six general cards at two levels
		// each and two exclusives at one level each, so fourteen selections
		// exhaust the pool. levelUpAllCards() drives that deterministically
		// instead of levelling card by card and hoping each call succeeds.
		int roundsUsed = 0;
		CHECK_TRUE(
			"every eligible card reaches its cap",
			levelUpAllCards(
				system,
				eligibleCards(system),
				randomEngine,
				roundsUsed));
		CHECK_EQ(
			"fourteen selections exhaust the ArcLink pool",
			14,
			roundsUsed);

		bool everyCardMaxed = true;
		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto card = static_cast<
				RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsValid(card) ||
				!rogueliteStrengtheningFitsCore(
					card,
					system.selectedCore())) {
				continue;
			}
			if (system.level(card) <
				rogueliteMaximumLevel(card)) {
				everyCardMaxed = false;
				CHECK_TRUE(
					std::string("eligible card still maxed: ") +
						cardLabel(card),
					false);
			}
		}

		CHECK_TRUE(
			"exhaustion check ran against a fully maxed build",
			everyCardMaxed);
		if (everyCardMaxed) {
			CHECK_FALSE(
				"exhausted pool yields no offers",
				system.generateStrengtheningOffers(
					randomEngine));
			CHECK_EQ(
				"exhausted generation leaves an empty panel",
				0,
				system.offerCount());
			CHECK_EQ(
				"exhausted generation keeps the core selected",
				static_cast<int>(RogueliteCoreType::ArcLink),
				static_cast<int>(system.selectedCore()));
			const RogueliteStats remaining =
				system.stats(GameplayConfig{});
			CHECK_TRUE(
				"exhausted generation keeps the build intact",
				remaining.arcLink.enabled);
		}
	}

	// ---------------------------------------------------------------------
	// Determinism and no implicit re-rolls
	// ---------------------------------------------------------------------

	void checkDeterminismAndNoImplicitReroll() {
		RogueliteUpgradeSystem first{};
		RogueliteUpgradeSystem second{};
		std::mt19937 firstEngine{ baseSeed };
		std::mt19937 secondEngine{ baseSeed };

		CHECK_TRUE(
			"first core established",
			establishCore(
				first,
				RogueliteCoreType::FocusFire,
				firstEngine));
		CHECK_TRUE(
			"second core established",
			establishCore(
				second,
				RogueliteCoreType::FocusFire,
				secondEngine));

		for (int round = 0; round < 6; ++round) {
			const bool firstGenerated =
				first.generateStrengtheningOffers(firstEngine);
			const bool secondGenerated =
				second.generateStrengtheningOffers(
					secondEngine);
			CHECK_EQ(
				std::string("generation outcome matches on round ") +
					std::to_string(round),
				firstGenerated,
				secondGenerated);
			if (!firstGenerated) {
				break;
			}
			CHECK_EQ(
				std::string("offer count matches on round ") +
					std::to_string(round),
				first.offerCount(),
				second.offerCount());
			for (int index = 0;
				index < first.offerCount();
				++index) {
				CHECK_EQ(
					std::string("offer matches on round ") +
						std::to_string(round) +
						" slot " + std::to_string(index),
					static_cast<int>(
						first.offerAt(index).strengthening),
					static_cast<int>(
						second.offerAt(index).strengthening));
			}
			if (!first.applyOffer(0).accepted ||
				!second.applyOffer(0).accepted) {
				break;
			}
		}

		// Repeating generation while a panel is pending must return that same
		// panel.
		RogueliteUpgradeSystem pending{};
		std::mt19937 pendingEngine{ baseSeed };
		CHECK_TRUE(
			"pending core established",
			establishCore(
				pending,
				RogueliteCoreType::PiercingRounds,
				pendingEngine));
		CHECK_TRUE(
			"pending panel generated",
			pending.generateStrengtheningOffers(pendingEngine));
		std::array<RogueliteStrengtheningType, 3> firstPanel{};
		for (int index = 0; index < pending.offerCount(); ++index) {
			firstPanel[static_cast<std::size_t>(index)] =
				pending.offerAt(index).strengthening;
		}

		// Repeating generation while a panel is pending must return that same
		// panel. Choosing a core already counted as a selection, so the total
		// is captured rather than assumed to be zero.
		const int selectionsBeforeRepeat =
			pending.selectionCounts().total();
		const auto pendingBeforeRepeat = pending;
		CHECK_TRUE(
			"repeating generation succeeds",
			pending.generateStrengtheningOffers(pendingEngine));
		for (int index = 0; index < pending.offerCount(); ++index) {
			CHECK_EQ(
				std::string("repeated generation keeps slot ") +
					std::to_string(index),
				static_cast<int>(
					firstPanel[static_cast<std::size_t>(index)]),
				static_cast<int>(
					pending.offerAt(index).strengthening));
		}
		CHECK_EQ(
			"repeated generation does not consume a selection",
			selectionsBeforeRepeat,
			pending.selectionCounts().total());
		CHECK_EQ(
			"repeated generation keeps the core selection count",
			1,
			pending.selectionCounts().coreSelections);
		CHECK_EQ(
			"repeated generation adds no strengthening selection",
			0,
			pending.selectionCounts().strengtheningSelections);
		checkStateEqual(
			"repeated generation changes nothing",
			pendingBeforeRepeat,
			pending);
		CHECK_TRUE(
			"the retained panel is still applicable",
			pending.applyOffer(0).accepted);
	}

	// ---------------------------------------------------------------------
	// Illegal calls leave the build untouched
	// ---------------------------------------------------------------------

	void checkIllegalCallsAreRejected() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };

		CHECK_EQ(
			"offerAt(-1) returns an empty offer",
			static_cast<int>(RogueliteOptionKind::None),
			static_cast<int>(system.offerAt(-1).kind));
		CHECK_EQ(
			"offerAt(99) returns an empty offer",
			static_cast<int>(RogueliteOptionKind::None),
			static_cast<int>(system.offerAt(99).kind));
		CHECK_FALSE(
			"applyOffer(-1) is refused",
			system.applyOffer(-1).accepted);
		CHECK_FALSE(
			"applyOffer(99) is refused",
			system.applyOffer(99).accepted);
		CHECK_EQ(
			"refused apply leaves the selection count at zero",
			0,
			system.selectionCounts().total());

		// Applying a strengthening before any core exists.
		CHECK_TRUE(
			"core panel generated",
			system.generateCoreOffers(randomEngine));
		// A pending core offer now previews the complete build that choosing
		// that core would produce, so this must succeed instead of being
		// refused as it was before core previews were supported.
		CHECK_TRUE(
			"core offer previews the chosen core build",
			[&]() {
				RogueliteStats preview{};
				return system.statsAfter(
					GameplayConfig{},
					0,
					preview);
			}());
		CHECK_FALSE(
			"invalid preview index is refused",
			[&]() {
				RogueliteStats preview{};
				return system.statsAfter(
					GameplayConfig{},
					-1,
					preview);
			}());

		// Slot 1 is the second entry of the fixed core order, which is
		// PiercingRounds: record what is actually offered instead of assuming
		// the first core.
		const auto offeredCore = system.offerAt(1).core;
		CHECK_EQ(
			"slot 1 offers the second core in the fixed order",
			static_cast<int>(RogueliteCoreType::PiercingRounds),
			static_cast<int>(offeredCore));
		CHECK_TRUE(
			"core applied",
			system.applyOffer(1).accepted);
		CHECK_EQ(
			"the applied core is the recorded offer",
			static_cast<int>(offeredCore),
			static_cast<int>(system.selectedCore()));
		CHECK_EQ(
			"the applied core is the last applied core",
			static_cast<int>(offeredCore),
			static_cast<int>(system.lastAppliedCore()));
		const auto appliedState = system;
		CHECK_FALSE(
			"re-applying the same selection is refused",
			system.applyOffer(1).accepted);
		CHECK_EQ(
			"refused re-apply does not add a selection",
			1,
			system.selectionCounts().total());
		CHECK_EQ(
			"refused re-apply cannot change the core",
			static_cast<int>(offeredCore),
			static_cast<int>(system.selectedCore()));
		CHECK_EQ(
			"refused re-apply cannot change the last applied core",
			static_cast<int>(offeredCore),
			static_cast<int>(system.lastAppliedCore()));
		checkStateEqual(
			"refused re-apply changes nothing",
			appliedState,
			system);
		CHECK_FALSE(
			"no strengthening was granted by the refused call",
			system.level(
				RogueliteStrengtheningType::DamageBoost) > 0);
	}

	// ---------------------------------------------------------------------
	// Parameters
	// ---------------------------------------------------------------------

	void checkBaseParameters() {
		RogueliteUpgradeSystem system{};
		const GameplayConfig config{};
		const RogueliteStats stats = system.stats(config);

		CHECK_NEAR(
			"no core: damage multiplier is neutral",
			1.0f,
			stats.damageMultiplier);
		CHECK_NEAR(
			"no core: fire interval is the configured value",
			config.fireInterval,
			stats.fireInterval);
		CHECK_EQ(
			"no core: magazine is the configured value",
			config.magazineCapacity,
			stats.magazineCapacity);
		CHECK_NEAR(
			"no core: reload is the configured value",
			config.reloadDuration,
			stats.reloadDuration);
		CHECK_EQ(
			"no core: health is the configured value",
			config.playerInitialHealth,
			stats.maximumHealth);
		CHECK_NEAR(
			"no core: move speed is the configured value",
			config.playerSpeed,
			stats.moveSpeed);
		CHECK_NEAR(
			"no core: spread is the configured value",
			config.playerProjectileSpreadDegrees,
			stats.projectileSpreadDegrees);
		CHECK_NEAR(
			"no core: projectile speed is the configured value",
			config.playerProjectileSpeed,
			stats.projectileSpeed);

		CHECK_FALSE(
			"no core: focus fire is disabled",
			stats.focusFire.enabled);
		CHECK_FALSE(
			"no core: piercing rounds are disabled",
			stats.piercingRounds.enabled);
		CHECK_FALSE(
			"no core: arc link is disabled",
			stats.arcLink.enabled);
	}

	void checkGeneralParameterLevels() {
		const GameplayConfig config{};
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for general parameters",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));

		for (const RogueliteStrengtheningType card : {
			RogueliteStrengtheningType::DamageBoost,
			RogueliteStrengtheningType::FireRate,
			RogueliteStrengtheningType::AmmoSystem,
			RogueliteStrengtheningType::ArmorCore,
			RogueliteStrengtheningType::MobilityCalibration,
			RogueliteStrengtheningType::BallisticCalibration
			}) {
			CHECK_TRUE(
				std::string("card maxed for parameters: ") +
					cardLabel(card),
				levelUpCardFully(system, card, randomEngine));
		}

		const RogueliteStats stats = system.stats(config);
		CHECK_NEAR(
			"level two damage multiplier",
			1.0f + 0.15f * 2.0f,
			stats.damageMultiplier);
		CHECK_NEAR(
			"level two fire interval",
			config.fireInterval / (1.0f + 0.10f * 2.0f),
			stats.fireInterval);
		CHECK_EQ(
			"level two magazine",
			config.magazineCapacity + 3 * 2,
			stats.magazineCapacity);
		CHECK_NEAR(
			"level two reload",
			config.reloadDuration - 0.10f * 2.0f,
			stats.reloadDuration);
		CHECK_EQ(
			"level two maximum health",
			config.playerInitialHealth + 15 * 2,
			stats.maximumHealth);
		CHECK_NEAR(
			"level two move speed",
			config.playerSpeed * (1.0f + 0.08f * 2.0f),
			stats.moveSpeed);
		CHECK_NEAR(
			"level two spread",
			config.playerProjectileSpreadDegrees - 1.0f * 2.0f,
			stats.projectileSpreadDegrees);
		CHECK_NEAR(
			"level two projectile speed",
			config.playerProjectileSpeed + 60.0f * 2.0f,
			stats.projectileSpeed);

		// Single level values, to prove the per-level step and not just the
		// two-level total. takeOneLevel stops after exactly one upgrade;
		// takeCard would max the card and make this check meaningless.
		RogueliteUpgradeSystem single{};
		std::mt19937 singleEngine{ baseSeed };
		CHECK_TRUE(
			"core established for single level",
			establishCore(
				single,
				RogueliteCoreType::FocusFire,
				singleEngine));
		CHECK_TRUE(
			"Damage Boost level one taken",
			takeOneLevel(
				single,
				RogueliteStrengtheningType::DamageBoost,
				singleEngine));
		CHECK_EQ(
			"Damage Boost is at exactly level one",
			1,
			single.level(
				RogueliteStrengtheningType::DamageBoost));
		CHECK_NEAR(
			"level one damage multiplier",
			1.15f,
			single.stats(config).damageMultiplier);
	}

	void checkFinalParametersDoNotDependOnOrder() {
		const GameplayConfig config{};
		const std::array<RogueliteStrengtheningType, 6> target{ {
			RogueliteStrengtheningType::DamageBoost,
			RogueliteStrengtheningType::DamageBoost,
			RogueliteStrengtheningType::FireRate,
			RogueliteStrengtheningType::FireRate,
			RogueliteStrengtheningType::AmmoSystem,
			RogueliteStrengtheningType::AmmoSystem
		} };
		const std::array<RogueliteStrengtheningType, 6> reversed{ {
			RogueliteStrengtheningType::AmmoSystem,
			RogueliteStrengtheningType::AmmoSystem,
			RogueliteStrengtheningType::FireRate,
			RogueliteStrengtheningType::FireRate,
			RogueliteStrengtheningType::DamageBoost,
			RogueliteStrengtheningType::DamageBoost
		} };

		RogueliteUpgradeSystem forward{};
		RogueliteUpgradeSystem backward{};
		std::mt19937 forwardEngine{ baseSeed };
		std::mt19937 backwardEngine{ baseSeed + 7u };

		CHECK_TRUE(
			"forward build core established",
			establishCore(
				forward,
				RogueliteCoreType::ArcLink,
				forwardEngine));
		CHECK_TRUE(
			"backward build core established",
			establishCore(
				backward,
				RogueliteCoreType::ArcLink,
				backwardEngine));

		for (const auto card : target) {
			CHECK_TRUE(
				std::string("forward build took: ") +
					cardLabel(card),
				takeCard(forward, card, forwardEngine));
		}
		for (const auto card : reversed) {
			CHECK_TRUE(
				std::string("backward build took: ") +
					cardLabel(card),
				takeCard(backward, card, backwardEngine));
		}

		const RogueliteStats first = forward.stats(config);
		const RogueliteStats second = backward.stats(config);
		CHECK_NEAR(
			"damage multiplier is order independent",
			first.damageMultiplier,
			second.damageMultiplier);
		CHECK_NEAR(
			"fire interval is order independent",
			first.fireInterval,
			second.fireInterval);
		CHECK_EQ(
			"magazine is order independent",
			first.magazineCapacity,
			second.magazineCapacity);
		CHECK_NEAR(
			"reload is order independent",
			first.reloadDuration,
			second.reloadDuration);
		CHECK_NEAR(
			"move speed is order independent",
			first.moveSpeed,
			second.moveSpeed);
	}

	void checkCoreParameters() {
		const GameplayConfig config{};

		// Focus fire, base and both exclusives.
		{
			RogueliteUpgradeSystem system{};
			std::mt19937 randomEngine{ baseSeed };
			CHECK_TRUE(
				"focus fire core established",
				establishCore(
					system,
					RogueliteCoreType::FocusFire,
					randomEngine));
			const RogueliteStats base = system.stats(config);
			CHECK_TRUE("focus fire enabled", base.focusFire.enabled);
			CHECK_NEAR(
				"focus fire base per stack",
				0.10f,
				base.focusFire.damagePerStack);
			CHECK_EQ(
				"focus fire base maximum stacks",
				3,
				base.focusFire.maximumStacks);
			CHECK_NEAR(
				"focus fire base expiry",
				1.5f,
				base.focusFire.expirySeconds);
			CHECK_FALSE(
				"focus fire does not retain by default",
				base.focusFire.retainsOnKill);

			CHECK_TRUE(
				"Deep Focus taken",
				takeCard(
					system,
					RogueliteStrengtheningType::DeepFocus,
					randomEngine));
			const RogueliteStats deep = system.stats(config);
			CHECK_NEAR(
				"Deep Focus per stack",
				0.15f,
				deep.focusFire.damagePerStack);
			CHECK_EQ(
				"Deep Focus maximum stacks",
				3,
				deep.focusFire.maximumStacks);

			CHECK_TRUE(
				"Lock Momentum taken",
				takeCard(
					system,
					RogueliteStrengtheningType::LockMomentum,
					randomEngine));
			const RogueliteStats both = system.stats(config);
			CHECK_TRUE(
				"Lock Momentum enables retention",
				both.focusFire.retainsOnKill);
			CHECK_NEAR(
				"Lock Momentum retention window",
				1.5f,
				both.focusFire.retentionSeconds);
			CHECK_NEAR(
				"both exclusives keep the Deep Focus step",
				0.15f,
				both.focusFire.damagePerStack);
			CHECK_FALSE(
				"focus fire parameters do not leak into piercing",
				both.piercingRounds.enabled);
			CHECK_FALSE(
				"focus fire parameters do not leak into arc link",
				both.arcLink.enabled);
		}

		// Piercing rounds, base and both exclusives.
		{
			RogueliteUpgradeSystem system{};
			std::mt19937 randomEngine{ baseSeed };
			CHECK_TRUE(
				"piercing core established",
				establishCore(
					system,
					RogueliteCoreType::PiercingRounds,
					randomEngine));
			const RogueliteStats base = system.stats(config);
			CHECK_TRUE(
				"piercing enabled",
				base.piercingRounds.enabled);
			CHECK_NEAR(
				"piercing first target multiplier",
				1.10f,
				base.piercingRounds.firstTargetMultiplier);
			CHECK_EQ(
				"piercing extra target limit",
				1,
				base.piercingRounds.maximumExtraTargets);
			CHECK_NEAR(
				"piercing extra target multiplier",
				0.60f,
				base.piercingRounds.extraTargetMultiplier);

			CHECK_TRUE(
				"Deep Penetration taken",
				takeCard(
					system,
					RogueliteStrengtheningType::DeepPenetration,
					randomEngine));
			CHECK_EQ(
				"Deep Penetration extra target limit",
				2,
				system.stats(config)
					.piercingRounds.maximumExtraTargets);

			CHECK_TRUE(
				"Penetration Amplifier taken",
				takeCard(
					system,
					RogueliteStrengtheningType::
						PenetrationAmplifier,
					randomEngine));
			const RogueliteStats both = system.stats(config);
			CHECK_NEAR(
				"Penetration Amplifier extra target multiplier",
				0.85f,
				both.piercingRounds.extraTargetMultiplier);
			CHECK_NEAR(
				"piercing first target multiplier is unchanged",
				1.10f,
				both.piercingRounds.firstTargetMultiplier);
		}

		// Arc link, base and both exclusives.
		{
			RogueliteUpgradeSystem system{};
			std::mt19937 randomEngine{ baseSeed };
			CHECK_TRUE(
				"arc link core established",
				establishCore(
					system,
					RogueliteCoreType::ArcLink,
					randomEngine));
			const RogueliteStats base = system.stats(config);
			CHECK_TRUE("arc link enabled", base.arcLink.enabled);
			CHECK_EQ(
				"arc link hits per discharge",
				3,
				base.arcLink.hitsPerDischarge);
			CHECK_NEAR(
				"arc link primary multiplier",
				0.40f,
				base.arcLink.primaryTargetMultiplier);
			CHECK_EQ(
				"arc link chain target limit",
				2,
				base.arcLink.maximumChainTargets);
			CHECK_NEAR(
				"arc link chain multiplier",
				0.40f,
				base.arcLink.chainMultiplier);
			CHECK_NEAR(
				"arc link chain radius",
				150.0f,
				base.arcLink.chainRadiusPixels);

			CHECK_TRUE(
				"Extended Circuit taken",
				takeCard(
					system,
					RogueliteStrengtheningType::ExtendedCircuit,
					randomEngine));
			CHECK_EQ(
				"Extended Circuit chain target limit",
				3,
				system.stats(config)
					.arcLink.maximumChainTargets);

			CHECK_TRUE(
				"Concentrated Discharge taken",
				takeCard(
					system,
					RogueliteStrengtheningType::
						ConcentratedDischarge,
					randomEngine));
			const RogueliteStats both = system.stats(config);
			CHECK_NEAR(
				"Concentrated Discharge primary multiplier",
				0.80f,
				both.arcLink.primaryTargetMultiplier);
			CHECK_NEAR(
				"arc link chain multiplier is unchanged",
				0.40f,
				both.arcLink.chainMultiplier);
			CHECK_NEAR(
				"arc link chain radius is unchanged",
				150.0f,
				both.arcLink.chainRadiusPixels);
		}
	}

	void checkPreviewMatchesSelection() {
		const GameplayConfig config{};
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for preview",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));
		CHECK_TRUE(
			"panel generated for preview",
			system.generateStrengtheningOffers(randomEngine));

		std::mt19937 probeEngine = randomEngine;
		for (int index = 0; index < system.offerCount(); ++index) {
			RogueliteStats preview{};
			CHECK_TRUE(
				std::string("preview succeeds for slot ") +
					std::to_string(index),
				system.statsAfter(config, index, preview));
			CHECK_EQ(
				std::string("preview does not consume randomness "
					"for slot ") + std::to_string(index),
				randomEngine(),
				probeEngine());
		}

		// Applying the previewed slot must produce exactly the previewed
		// parameters.
		RogueliteStats preview{};
		CHECK_TRUE(
			"preview available for slot zero",
			system.statsAfter(config, 0, preview));
		CHECK_TRUE(
			"previewed slot applies",
			system.applyOffer(0).accepted);
		const RogueliteStats actual = system.stats(config);

		CHECK_NEAR(
			"previewed damage multiplier matches",
			preview.damageMultiplier,
			actual.damageMultiplier);
		CHECK_NEAR(
			"previewed fire interval matches",
			preview.fireInterval,
			actual.fireInterval);
		CHECK_EQ(
			"previewed magazine matches",
			preview.magazineCapacity,
			actual.magazineCapacity);
		CHECK_NEAR(
			"previewed reload matches",
			preview.reloadDuration,
			actual.reloadDuration);
		CHECK_EQ(
			"previewed health matches",
			preview.maximumHealth,
			actual.maximumHealth);
		CHECK_NEAR(
			"previewed move speed matches",
			preview.moveSpeed,
			actual.moveSpeed);
		CHECK_NEAR(
			"previewed spread matches",
			preview.projectileSpreadDegrees,
			actual.projectileSpreadDegrees);
		CHECK_NEAR(
			"previewed projectile speed matches",
			preview.projectileSpeed,
			actual.projectileSpeed);
		CHECK_TRUE(
			"previewed core bonus matches",
			preview.focusFire.enabled ==
				actual.focusFire.enabled);
	}

	void checkArmorRestoreHappensOncePerSelection() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for armor restore",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));

		// Drive the build to the first ArmorCore selection. ArmorCore only
		// leaves the pool once it is fully levelled, so iterating panels is
		// guaranteed to reach it; the bound is a safety net, and the final
		// check reports failure instead of silently passing when it is hit.
		int restored = -1;
		for (int round = 0; round < 24 && restored < 0; ++round) {
			if (!system.generateStrengtheningOffers(randomEngine)) {
				break;
			}
			int armorSlot = -1;
			for (int index = 0; index < system.offerCount(); ++index) {
				if (system.offerAt(index).strengthening ==
					RogueliteStrengtheningType::ArmorCore) {
					armorSlot = index;
					break;
				}
			}
			if (armorSlot >= 0) {
				const RogueliteApplyResult applied =
					system.applyOffer(armorSlot);
				CHECK_TRUE(
					"armor core selection is accepted",
					applied.accepted);
				restored = applied.restoredHealth;
				CHECK_EQ(
					"armor core level after selection",
					1,
					system.level(
						RogueliteStrengtheningType::
							ArmorCore));
				break;
			}
			if (!system.applyOffer(0).accepted) {
				break;
			}
		}
		CHECK_EQ(
			"armor core appeared and restored 15 health once",
			RogueliteTuning::armorCoreRestoredHealth,
			restored);
	}

	void checkSixStepSequenceDoesNotStopEarly() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for the six step sequence",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));

		for (int step = 0; step < 6; ++step) {
			const bool generated =
				system.generateStrengtheningOffers(randomEngine);
			CHECK_TRUE(
				std::string("step ") + std::to_string(step) +
					": a full panel is still available",
				generated);
			if (!generated) {
				return;
			}
			CHECK_EQ(
				std::string("step ") + std::to_string(step) +
					": panel holds three options",
				3,
				system.offerCount());
			CHECK_TRUE(
				std::string("step ") + std::to_string(step) +
					": selection applies",
				system.applyOffer(step % 3).accepted);
			// The core chosen during setup is a selection too, so the total
			// is always one ahead of the strengthening count.
			CHECK_EQ(
				std::string("step ") + std::to_string(step) +
					": the core selection is still counted once",
				1,
				system.selectionCounts().coreSelections);
			CHECK_EQ(
				std::string("step ") + std::to_string(step) +
					": strengthening selections counted",
				step + 1,
				system.selectionCounts().strengtheningSelections);
			CHECK_EQ(
				std::string("step ") + std::to_string(step) +
					": total selections counted",
				step + 2,
				system.selectionCounts().total());
		}
	}

	void checkResetClearsEverything() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for reset",
			establishCore(
				system,
				RogueliteCoreType::PiercingRounds,
				randomEngine));
		CHECK_TRUE(
			"panel generated for reset",
			system.generateStrengtheningOffers(randomEngine));
		CHECK_TRUE(
			"reset scenario applies an offer",
			system.applyOffer(0).accepted);
		CHECK_TRUE(
			"reset scenario has a non-zero selection count",
			system.selectionCounts().total() > 0);

		system.reset();

		CHECK_EQ(
			"reset clears the core",
			static_cast<int>(RogueliteCoreType::None),
			static_cast<int>(system.selectedCore()));
		CHECK_FALSE("reset clears hasCore", system.hasCore());
		CHECK_EQ(
			"reset clears the offer panel",
			0,
			system.offerCount());
		CHECK_EQ(
			"reset clears every selection count",
			0,
			system.selectionCounts().total());
		CHECK_EQ(
			"reset clears the core selection count",
			0,
			system.selectionCounts().coreSelections);
		CHECK_EQ(
			"reset clears the strengthening selection count",
			0,
			system.selectionCounts().strengtheningSelections);
		for (std::size_t index = 0;
			index < rogueliteStrengtheningTypeCount;
			++index) {
			const auto card = static_cast<
				RogueliteStrengtheningType>(index);
			if (!rogueliteStrengtheningIsValid(card)) {
				continue;
			}
			CHECK_EQ(
				std::string("reset clears level: ") +
					cardLabel(card),
				0,
				system.level(card));
		}

		const RogueliteStats cleared =
			system.stats(GameplayConfig{});
		CHECK_FALSE(
			"reset disables the piercing core bonus",
			cleared.piercingRounds.enabled);
		CHECK_NEAR(
			"reset restores the neutral damage multiplier",
			1.0f,
			cleared.damageMultiplier);

		// A reset system can pick a fresh core, including a different one.
		CHECK_TRUE(
			"a fresh core can be chosen after reset",
			establishCore(
				system,
				RogueliteCoreType::ArcLink,
				randomEngine));
		CHECK_EQ(
			"the new core is recorded",
			static_cast<int>(RogueliteCoreType::ArcLink),
			static_cast<int>(system.selectedCore()));
	}

	void checkInvalidCardQueriesAreSafe() {
		RogueliteUpgradeSystem system{};
		CHECK_EQ(
			"level of None is zero",
			0,
			system.level(RogueliteStrengtheningType::None));
		CHECK_EQ(
			"level of Count is zero",
			0,
			system.level(RogueliteStrengtheningType::Count));
		CHECK_EQ(
			"level of an out-of-range card is zero",
			0,
			system.level(static_cast<
				RogueliteStrengtheningType>(999)));
		CHECK_EQ(
			"last applied strengthening starts as None",
			static_cast<int>(RogueliteStrengtheningType::None),
			static_cast<int>(
				system.lastAppliedStrengthening()));
		CHECK_EQ(
			"last applied core starts as None",
			static_cast<int>(RogueliteCoreType::None),
			static_cast<int>(system.lastAppliedCore()));
	}

	void checkLastAppliedRecord() {
		RogueliteUpgradeSystem system{};
		std::mt19937 randomEngine{ baseSeed };
		CHECK_TRUE(
			"core established for last applied",
			establishCore(
				system,
				RogueliteCoreType::FocusFire,
				randomEngine));
		CHECK_EQ(
			"the core is the last applied selection",
			static_cast<int>(RogueliteCoreType::FocusFire),
			static_cast<int>(system.lastAppliedCore()));
		CHECK_TRUE(
			"panel generated for last applied",
			system.generateStrengtheningOffers(randomEngine));
		const auto picked =
			system.offerAt(0).strengthening;
		CHECK_TRUE(
			"strengthening applied",
			system.applyOffer(0).accepted);
		CHECK_EQ(
			"the card is the last applied selection",
			static_cast<int>(picked),
			static_cast<int>(
				system.lastAppliedStrengthening()));
		CHECK_EQ(
			"a strengthening selection clears the last core record",
			static_cast<int>(RogueliteCoreType::None),
			static_cast<int>(system.lastAppliedCore()));
	}

	// Sets the level mask for a card as it is previewed at its current level.
	void markCovered(
		std::array<unsigned int, rogueliteStrengtheningTypeCount>& covered,
		RogueliteStrengtheningType card,
		int levelAtOffer) {
		covered[static_cast<std::size_t>(card)] |=
			1u << levelAtOffer;
	}

	bool everyLevelCovered(
		const std::array<unsigned int, rogueliteStrengtheningTypeCount>& covered,
		RogueliteStrengtheningType card) {
		const unsigned int expected =
			(1u << rogueliteMaximumLevel(card)) - 1u;
		return covered[static_cast<std::size_t>(card)] == expected;
	}

	void checkFullPoolReferenceTopUp() {
		// Scenario -1: both exclusives are gone and all six general cards are
		// still unowned, so the panel must come entirely from the general pool.
		// Scenarios 0 and 1 additionally exhaust one general category, which
		// forces TWO top-up draws from the remaining pool.
		for (int scenario = -1; scenario < 2; ++scenario) {
			RogueliteUpgradeSystem build{};
			std::mt19937 setupEngine{ baseSeed };
			CHECK_TRUE("reference setup core", establishCore(build,
				RogueliteCoreType::FocusFire, setupEngine));
			for (int choice = 0; choice < 2; ++choice) {
				CHECK_TRUE("reference setup exclusive panel",
					build.generateStrengtheningOffers(setupEngine));
				int selected = -1;
				for (int slot = 0; slot < build.offerCount(); ++slot) {
					if (rogueliteStrengtheningIsExclusive(build.offerAt(slot).strengthening))
						selected = slot;
				}
				CHECK_TRUE("reference setup takes only exclusive",
					build.applyOffer(selected).accepted);
			}
			for (const auto card : generalCards())
				CHECK_EQ("six generals initially unowned", 0, build.level(card));
			if (scenario >= 0) {
				const auto exhaustedCategory = scenario == 0
					? RogueliteCategory::OffensiveSustain
					: RogueliteCategory::DefensiveControl;
				for (int choice = 0; choice < 6; ++choice) {
					CHECK_TRUE("category exhaustion panel",
						build.generateStrengtheningOffers(setupEngine));
					int selected = -1;
					for (int slot = 0; slot < build.offerCount(); ++slot) {
						if (rogueliteCategoryOf(build.offerAt(slot).strengthening) == exhaustedCategory)
							selected = slot;
					}
					CHECK_TRUE("take exhausted category", build.applyOffer(selected).accepted);
				}
				for (const auto card : generalCards()) {
					CHECK_EQ("only selected category is maxed",
						rogueliteCategoryOf(card) == exhaustedCategory ? 2 : 0,
						build.level(card));
				}
			}
			for (std::uint32_t seed = 0; seed < 32u; ++seed) {
				auto actual = build;
				std::mt19937 engine{ baseSeed + seed };
				auto expectedEngine = engine;
				const auto expected = referencePanel(build, expectedEngine);
				const std::string label = "full-pool scenario " + std::to_string(scenario)
					+ " seed " + std::to_string(seed);
				CHECK_TRUE(label, actual.generateStrengtheningOffers(engine));
				CHECK_EQ(label + ": count", static_cast<int>(expected.size()), actual.offerCount());
				for (int slot = 0; slot < actual.offerCount(); ++slot) {
					CHECK_EQ(label + ": sampled card",
						static_cast<int>(expected[static_cast<std::size_t>(slot)]),
						static_cast<int>(actual.offerAt(slot).strengthening));
				}
				CHECK_TRUE(label + ": engine matches reference", engine == expectedEngine);
				offerPanelIsLegal(actual, label);
			}
		}
	}

	void checkGenerationPreservesPendingState() {
		for (const bool strengthening : { false, true }) {
			RogueliteUpgradeSystem system{};
			std::mt19937 engine{ baseSeed };
			CHECK_TRUE("preservation setup", system.generateCoreOffers(engine));
			if (strengthening) {
				CHECK_TRUE("preservation core selected", system.applyOffer(0).accepted);
				CHECK_TRUE("preservation card taken", takeOneLevel(system,
					RogueliteStrengtheningType::DamageBoost, engine));
				CHECK_TRUE("preservation strengthening panel", system.generateStrengtheningOffers(engine));
			}
			const auto before = system;
			const auto beforeEngine = engine;
			for (int repeat = 0; repeat < 3; ++repeat) {
				CHECK_TRUE("same-kind repeated generation", strengthening
					? system.generateStrengtheningOffers(engine) : system.generateCoreOffers(engine));
				checkStateEqual("repeat preserves all state", before, system);
				CHECK_TRUE("repeat preserves engine", engine == beforeEngine);
				CHECK_FALSE("wrong-kind generation rejected", strengthening
					? system.generateCoreOffers(engine) : system.generateStrengtheningOffers(engine));
				checkStateEqual("rejection preserves all state", before, system);
				CHECK_TRUE("rejection preserves engine", engine == beforeEngine);
			}
			RogueliteStats preview{};
			CHECK_TRUE("retained panel previews", system.statsAfter(GameplayConfig{}, 0, preview));
			CHECK_TRUE("retained panel applies", system.applyOffer(0).accepted);
			checkStatsEqual("retained selection matches preview", preview, system.stats(GameplayConfig{}));
			CHECK_EQ("successful selection clears panel", 0, system.offerCount());
			CHECK_FALSE("duplicate selection refused", system.applyOffer(0).accepted);
		}
	}

	// Distinct sentinel values for every field of RogueliteStats, used to prove
	// that a refused preview leaves its output parameter completely untouched.
	RogueliteStats markedStats() {
		RogueliteStats value{};
		value.damageMultiplier = -11.0f;
		value.fireInterval = -12.0f;
		value.magazineCapacity = -13;
		value.reloadDuration = -14.0f;
		value.maximumHealth = -15;
		value.moveSpeed = -16.0f;
		value.projectileSpreadDegrees = -17.0f;
		value.projectileSpeed = -18.0f;
		value.focusFire = { true, -19.0f, -20, -21.0f, true, -22.0f };
		value.piercingRounds = { true, -23.0f, -24, -25.0f };
		value.arcLink = { true, -26, -27.0f, -28, -29.0f, -30.0f };
		return value;
	}

	void checkEveryCorePreview() {
		GameplayConfig config{};
		config.fireInterval = 0.37f;
		config.playerSpeed = 213.0f;
		RogueliteUpgradeSystem panel{};
		std::mt19937 engine{ baseSeed };
		CHECK_TRUE("core preview panel", panel.generateCoreOffers(engine));
		const auto before = panel;
		const auto beforeEngine = engine;
		for (int slot = 0; slot < panel.offerCount(); ++slot) {
			RogueliteStats preview = markedStats();
			CHECK_TRUE("every core previews", panel.statsAfter(config, slot, preview));
			checkStateEqual("core preview read-only", before, panel, config);
			CHECK_TRUE("core preview keeps engine", engine == beforeEngine);
			auto selected = panel;
			CHECK_TRUE("previewed core applies", selected.applyOffer(slot).accepted);
			checkStatsEqual("complete core preview", selected.stats(config), preview);
			for (const int invalid : { -1, 3, 99 }) {
				const auto marker = markedStats();
				RogueliteStats untouched = marker;
				CHECK_FALSE("invalid core index rejected",
					selected.statsAfter(config, invalid, untouched));
				checkStatsEqual("failed preview leaves all output fields", marker, untouched);
			}
		}
		for (const int invalid : { -1, 3, 99 }) {
			const auto marker = markedStats();
			RogueliteStats untouched = marker;
			CHECK_FALSE("invalid core index rejected", panel.statsAfter(config, invalid, untouched));
			checkStatsEqual("invalid index leaves output", marker, untouched);
			checkStateEqual("invalid index leaves build", before, panel, config);
		}
	}

	void checkEveryStrengtheningPreview() {
		GameplayConfig config{};
		config.fireInterval = 0.37f;
		config.reloadDuration = 0.05f; // Preview must reproduce clamping too.
		config.playerProjectileSpreadDegrees = 0.5f;
		for (const auto core : rogueliteCoreOfferOrder()) {
			RogueliteUpgradeSystem system{};
			std::mt19937 engine{ baseSeed };
			std::array<unsigned int, rogueliteStrengtheningTypeCount> covered{};
			CHECK_TRUE("strengthening preview core", establishCore(system, core, engine));
			for (int round = 0; round < 16; ++round) {
				if (!system.generateStrengtheningOffers(engine)) break;
				const auto before = system;
				const auto beforeEngine = engine;
				for (int slot = 0; slot < system.offerCount(); ++slot) {
					const auto card = system.offerAt(slot).strengthening;
					RogueliteStats preview = markedStats();
					CHECK_TRUE("every strengthening previews",
						system.statsAfter(config, slot, preview));
					checkStateEqual("strengthening preview read-only", before, system, config);
					CHECK_TRUE("strengthening preview keeps engine", engine == beforeEngine);
					auto selected = system;
					CHECK_TRUE("previewed strengthening applies", selected.applyOffer(slot).accepted);
					checkStatsEqual("complete strengthening preview",
						selected.stats(config), preview);
					markCovered(covered, card, system.level(card));
				}
				for (const int invalid : { -1, system.offerCount(), 99 }) {
					const auto marker = markedStats();
					RogueliteStats untouched = marker;
					CHECK_FALSE("invalid strengthening index",
						system.statsAfter(config, invalid, untouched));
					checkStatsEqual("invalid strengthening output unchanged", marker, untouched);
					checkStateEqual("invalid strengthening state unchanged", before, system, config);
				}
				CHECK_TRUE("advance preview coverage", system.applyOffer(0).accepted);
			}
			for (const auto card : eligibleCards(system)) {
				CHECK_TRUE("all legal card levels previewed: " + cardLabel(card),
					everyLevelCovered(covered, card));
			}
		}
	}

	void runAll() {
		NEON_TEST_CASE(checkFullPoolReferenceTopUp);
		NEON_TEST_CASE(checkGenerationPreservesPendingState);
		NEON_TEST_CASE(checkEveryCorePreview);
		NEON_TEST_CASE(checkEveryStrengtheningPreview);
		NEON_TEST_CASE(checkInitialCoreOffers);
		NEON_TEST_CASE(checkCoreGenerationDoesNotConsumeRandomness);
		NEON_TEST_CASE(checkStrengtheningNeedsACore);
		NEON_TEST_CASE(checkCoreCanOnlyBeChosenOnce);
		NEON_TEST_CASE(checkCorePoolsAreSeparated);
		NEON_TEST_CASE(checkGeneralCardsAreExcludedWhenMaxed);
		NEON_TEST_CASE(checkExclusiveCardsAreNeverRepeated);
		NEON_TEST_CASE(checkNormalPanelGuaranteesEveryCategory);
		NEON_TEST_CASE(checkExclusiveExhaustionIsHandled);
		NEON_TEST_CASE(checkCategoryExhaustionTopsUpFromRemainingCards);
		NEON_TEST_CASE(checkFullExhaustionIsHandled);
		NEON_TEST_CASE(checkDeterminismAndNoImplicitReroll);
		NEON_TEST_CASE(checkIllegalCallsAreRejected);
		NEON_TEST_CASE(checkBaseParameters);
		NEON_TEST_CASE(checkGeneralParameterLevels);
		NEON_TEST_CASE(checkFinalParametersDoNotDependOnOrder);
		NEON_TEST_CASE(checkCoreParameters);
		NEON_TEST_CASE(checkPreviewMatchesSelection);
		NEON_TEST_CASE(checkArmorRestoreHappensOncePerSelection);
		NEON_TEST_CASE(checkSixStepSequenceDoesNotStopEarly);
		NEON_TEST_CASE(checkResetClearsEverything);
		NEON_TEST_CASE(checkInvalidCardQueriesAreSafe);
		NEON_TEST_CASE(checkLastAppliedRecord);
	}

}//namespace

NEON_TEST_MAIN()
