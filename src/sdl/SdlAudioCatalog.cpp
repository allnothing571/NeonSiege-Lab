#include "sdl/SdlAudioCatalog.h"

namespace neon::sdl {

	const std::array<
		SoundAssetDefinition,
		soundAssetCount
	>& soundAssetDefinitions() noexcept {
		static constexpr std::array<
			SoundAssetDefinition,
			soundAssetCount
		> definitions{{
			{
				SoundId::UiSelect,
				"audio/ui/select.wav",
				55,
				80
			},
			{
				SoundId::UiConfirm,
				"audio/ui/confirm.wav",
				70,
				0
			},
			{
				SoundId::UiBack,
				"audio/ui/back.wav",
				70,
				0
			},
			{
				SoundId::PlayerShot,
				"audio/combat/player_shot.wav",
				75,
				0
			},
			{
				SoundId::EnemyShot,
				"audio/combat/enemy_shot.wav",
				60,
				50
			},
			{
				SoundId::ProjectileHit,
				"audio/combat/projectile_hit.wav",
				65,
				35
			},
			{
				SoundId::PlayerDamaged,
				"audio/combat/player_damaged.wav",
				90,
				0
			},
			{
				SoundId::EnemyDied,
				"audio/combat/enemy_died.wav",
				80,
				50
			},
			{
				SoundId::ReloadStarted,
				"audio/combat/reload_start.wav",
				70,
				0
			},
			{
				SoundId::ReloadCompleted,
				"audio/combat/reload_complete.wav",
				80,
				0
			},
			{
				SoundId::WaveStarted,
				"audio/system/wave_start.wav",
				85,
				0
			},
			{
				SoundId::UpgradeSelected,
				"audio/system/upgrade_selected.wav",
				85,
				0
			},
			{
				SoundId::Victory,
				"audio/system/victory.wav",
				100,
				0
			},
			{
				SoundId::GameOver,
				"audio/system/game_over.wav",
				100,
				0
			}
		}};

		return definitions;
	}

	const SoundAssetDefinition& soundAssetFor(
		SoundId id) noexcept {
		const auto& definitions =
			soundAssetDefinitions();
		const std::size_t index =
			static_cast<std::size_t>(id);

		if (index >= definitions.size()) {
			return definitions.front();
		}

		return definitions[index];
	}

}//namespace neon::sdl
