#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace neon::sdl {

	enum class SoundId : std::uint8_t {
		UiSelect,
		UiConfirm,
		UiBack,
		PlayerShot,
		EnemyShot,
		ProjectileHit,
		PlayerDamaged,
		EnemyDied,
		ReloadStarted,
		ReloadCompleted,
		WaveStarted,
		UpgradeSelected,
		Victory,
		GameOver,
		Count
	};

	struct SoundAssetDefinition {
		SoundId id = SoundId::UiSelect;
		std::string_view relativePath{};
		int baseVolumePercent = 100;
		std::uint32_t cooldownMilliseconds = 0;
	};

	inline constexpr std::size_t soundAssetCount =
		static_cast<std::size_t>(SoundId::Count);

	const std::array<
		SoundAssetDefinition,
		soundAssetCount
	>& soundAssetDefinitions() noexcept;

	const SoundAssetDefinition& soundAssetFor(
		SoundId id
	) noexcept;

}//namespace neon::sdl
