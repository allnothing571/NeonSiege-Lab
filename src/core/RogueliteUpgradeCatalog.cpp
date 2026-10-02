#include "core/RogueliteUpgradeCatalog.h"

#include <cstddef>

namespace neon {

	const std::array<
		RogueliteCoreDefinition,
		rogueliteCoreTypeCount
	>& rogueliteCoreDefinitions() noexcept {
		static constexpr std::array<
			RogueliteCoreDefinition,
			rogueliteCoreTypeCount
		> definitions{{
			{
				RogueliteCoreType::None,
				"无核心",
				"no core",
				"尚未选择核心机制。",
				"No core mechanism selected yet."
			},
			{
				RogueliteCoreType::FocusFire,
				"聚焦射击",
				"Focus Fire",
				"连续命中同一目标逐步提升伤害，"
				"中断 1.5 秒后重置。",
				"Consecutive hits on one target build "
				"damage, resetting after 1.5 seconds "
				"without a hit."
			},
			{
				RogueliteCoreType::PiercingRounds,
				"贯穿弹道",
				"Piercing Rounds",
				"首个目标承受 1.10 倍伤害，"
				"弹体可继续贯穿 1 个额外目标，"
				"额外目标承受 0.60 倍伤害。",
				"The first target takes 1.10x damage; "
				"the projectile carries on through 1 "
				"extra target for 0.60x damage."
			},
			{
				RogueliteCoreType::ArcLink,
				"电弧联动",
				"Arc Link",
				"每 3 次普通弹体命中触发一次电弧，"
				"主目标追加 0.40 倍伤害，"
				"并向 150 像素内最多 2 个目标连锁，"
				"连锁造成 0.40 倍伤害。",
				"Every 3rd ordinary projectile hit "
				"discharges an arc: +0.40x on the main "
				"target and 0.40x on up to 2 chained "
				"targets within 150 pixels."
			}
		}};

		return definitions;
	}

	const std::array<
		RogueliteStrengtheningDefinition,
		rogueliteStrengtheningTypeCount
	>& rogueliteStrengtheningDefinitions() noexcept {
		static constexpr std::array<
			RogueliteStrengtheningDefinition,
			rogueliteStrengtheningTypeCount
		> definitions{{
			{
				RogueliteStrengtheningType::None,
				RogueliteCategory::Exclusive,
				false,
				RogueliteCoreType::None,
				0,
				"无强化",
				"no strengthening",
				"占位项，不会出现在选项中。",
				"Placeholder entry that is never offered."
			},
			{
				RogueliteStrengtheningType::DeepFocus,
				RogueliteCategory::Exclusive,
				true,
				RogueliteCoreType::FocusFire,
				1,
				"深度聚焦",
				"Deep Focus",
				"聚焦射击每次积累 15% 伤害，"
				"上限提升到 45%。",
				"Focus Fire builds 15% damage per "
				"stack, raising the cap to 45%."
			},
			{
				RogueliteStrengtheningType::LockMomentum,
				RogueliteCategory::Exclusive,
				true,
				RogueliteCoreType::FocusFire,
				1,
				"锁定动能",
				"Lock Momentum",
				"击杀后把聚焦加成保留给下一个目标，"
				"保留 1.5 秒。",
				"On a kill, the focus bonus is kept for "
				"the next target for 1.5 seconds."
			},
			{
				RogueliteStrengtheningType::DeepPenetration,
				RogueliteCategory::Exclusive,
				true,
				RogueliteCoreType::PiercingRounds,
				1,
				"深度贯穿",
				"Deep Penetration",
				"贯穿额外目标上限提升到 2 个。",
				"Raises the extra target limit to 2."
			},
			{
				RogueliteStrengtheningType::PenetrationAmplifier,
				RogueliteCategory::Exclusive,
				true,
				RogueliteCoreType::PiercingRounds,
				1,
				"贯穿增幅",
				"Penetration Amplifier",
				"额外目标倍率提升到 0.85 倍。",
				"Raises the extra target multiplier to "
				"0.85x."
			},
			{
				RogueliteStrengtheningType::ExtendedCircuit,
				RogueliteCategory::Exclusive,
				true,
				RogueliteCoreType::ArcLink,
				1,
				"延伸回路",
				"Extended Circuit",
				"电弧连锁目标上限提升到 3 个。",
				"Raises the chain target limit to 3."
			},
			{
				RogueliteStrengtheningType::ConcentratedDischarge,
				RogueliteCategory::Exclusive,
				true,
				RogueliteCoreType::ArcLink,
				1,
				"集中放电",
				"Concentrated Discharge",
				"电弧主目标追加倍率提升到 0.80 倍。",
				"Raises the arc's main target bonus to "
				"0.80x."
			},
			{
				RogueliteStrengtheningType::DamageBoost,
				RogueliteCategory::OffensiveSustain,
				false,
				RogueliteCoreType::None,
				2,
				"伤害强化",
				"Damage Boost",
				"每级提升 15% 基础伤害倍率。",
				"Each level adds 15% base damage."
			},
			{
				RogueliteStrengtheningType::FireRate,
				RogueliteCategory::OffensiveSustain,
				false,
				RogueliteCoreType::None,
				2,
				"射速提升",
				"Fire Rate",
				"每级把开火间隔缩短为 1/(1+10%)。",
				"Each level divides the fire interval by "
				"1 + 10%."
			},
			{
				RogueliteStrengtheningType::AmmoSystem,
				RogueliteCategory::OffensiveSustain,
				false,
				RogueliteCoreType::None,
				2,
				"弹药系统",
				"Ammo System",
				"每级弹匣 +3 发，换弹时间 -0.10 秒。",
				"Each level adds 3 rounds to the "
				"magazine and removes 0.10 seconds from "
				"the reload."
			},
			{
				RogueliteStrengtheningType::ArmorCore,
				RogueliteCategory::DefensiveControl,
				false,
				RogueliteCoreType::None,
				2,
				"装甲核心",
				"Armor Core",
				"每级最大生命 +15，"
				"选择时立即恢复 15 点生命。",
				"Each level adds 15 maximum health and "
				"heals 15 immediately on selection."
			},
			{
				RogueliteStrengtheningType::MobilityCalibration,
				RogueliteCategory::DefensiveControl,
				false,
				RogueliteCoreType::None,
				2,
				"机动校准",
				"Mobility Calibration",
				"每级移动速度提升 8%。",
				"Each level adds 8% movement speed."
			},
			{
				RogueliteStrengtheningType::BallisticCalibration,
				RogueliteCategory::DefensiveControl,
				false,
				RogueliteCoreType::None,
				2,
				"弹道校准",
				"Ballistic Calibration",
				"每级散布 -1 度、弹速 +60。",
				"Each level removes 1 degree of spread "
				"and adds 60 projectile speed."
			}
		}};

		return definitions;
	}

	const RogueliteCoreDefinition& rogueliteCoreDefinitionFor(
		RogueliteCoreType type) noexcept {
		const auto& definitions = rogueliteCoreDefinitions();
		const std::size_t index =
			static_cast<std::size_t>(type);
		if (index >= definitions.size()) {
			return definitions.front();
		}
		return definitions[index];
	}

	const RogueliteStrengtheningDefinition&
		rogueliteStrengtheningDefinitionFor(
			RogueliteStrengtheningType type) noexcept {
		const auto& definitions =
			rogueliteStrengtheningDefinitions();
		const std::size_t index =
			static_cast<std::size_t>(type);
		if (index >= definitions.size()) {
			return definitions.front();
		}
		return definitions[index];
	}

	bool rogueliteCoreIsValid(
		RogueliteCoreType type) noexcept {
		// Range checked rather than compared against the sentinels alone: an
		// out-of-range cast must not be treated as a usable core.
		const int index = static_cast<int>(type);
		return index > static_cast<int>(RogueliteCoreType::None) &&
			index < static_cast<int>(RogueliteCoreType::Count);
	}

	bool rogueliteStrengtheningIsValid(
		RogueliteStrengtheningType type) noexcept {
		// Range checked rather than compared against the sentinels alone: an
		// out-of-range cast must not be treated as a usable card.
		const int index = static_cast<int>(type);
		return index >
				static_cast<int>(RogueliteStrengtheningType::None) &&
			index < static_cast<int>(
				RogueliteStrengtheningType::Count);
	}

	bool rogueliteStrengtheningIsExclusive(
		RogueliteStrengtheningType type) noexcept {
		if (!rogueliteStrengtheningIsValid(type)) {
			return false;
		}
		return rogueliteStrengtheningDefinitionFor(type)
			.exclusive;
	}

	RogueliteCategory rogueliteCategoryOf(
		RogueliteStrengtheningType type) noexcept {
		if (!rogueliteStrengtheningIsValid(type)) {
			return RogueliteCategory::Count;
		}
		return rogueliteStrengtheningDefinitionFor(type)
			.category;
	}

	RogueliteCoreType rogueliteAttributedCoreOf(
		RogueliteStrengtheningType type) noexcept {
		if (!rogueliteStrengtheningIsExclusive(type)) {
			return RogueliteCoreType::None;
		}
		return rogueliteStrengtheningDefinitionFor(type)
			.attributedCore;
	}

	int rogueliteMaximumLevel(
		RogueliteStrengtheningType type) noexcept {
		if (!rogueliteStrengtheningIsValid(type)) {
			return 0;
		}
		return rogueliteStrengtheningDefinitionFor(type)
			.maximumLevel;
	}

	bool rogueliteStrengtheningFitsCore(
		RogueliteStrengtheningType type,
		RogueliteCoreType core) noexcept {
		if (!rogueliteStrengtheningIsValid(type) ||
			!rogueliteCoreIsValid(core)) {
			return false;
		}
		if (!rogueliteStrengtheningIsExclusive(type)) {
			return true;
		}
		return rogueliteAttributedCoreOf(type) == core;
	}

	const std::array<
		RogueliteCoreType,
		rogueliteSelectableCoreCount
	>& rogueliteCoreOfferOrder() noexcept {
		static constexpr std::array<
			RogueliteCoreType,
			rogueliteSelectableCoreCount
		> order{{
			RogueliteCoreType::FocusFire,
			RogueliteCoreType::PiercingRounds,
			RogueliteCoreType::ArcLink
		}};

		return order;
	}

	const char* rogueliteCategoryEnglishName(
		RogueliteCategory category) noexcept {
		switch (category) {
		case RogueliteCategory::Exclusive:
			return "Exclusive";
		case RogueliteCategory::OffensiveSustain:
			return "Offensive Sustain";
		case RogueliteCategory::DefensiveControl:
			return "Defensive Control";
		default:
			return "Unknown";
		}
	}

	const char* rogueliteCategoryChineseName(
		RogueliteCategory category) noexcept {
		switch (category) {
		case RogueliteCategory::Exclusive:
			return "专属";
		case RogueliteCategory::OffensiveSustain:
			return "输出续航";
		case RogueliteCategory::DefensiveControl:
			return "防御操控";
		default:
			return "未知";
		}
	}

}//namespace neon
