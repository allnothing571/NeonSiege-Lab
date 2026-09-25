#include "sdl/SdlAssetCatalog.h"

#include <SDL_image.h>

#include "sdl/SdlPaths.h"

namespace neon::sdl {

	const std::array<TextureAssetDefinition, 11>&
	textureAssetDefinitions() noexcept {
		static constexpr std::array<
			TextureAssetDefinition,
			11
		> definitions{
			TextureAssetDefinition{
				TextureId::BackgroundCrossfire,
				"textures/maps/crossfire/background.png"
			},
			TextureAssetDefinition{
				TextureId::BackgroundSplitCorridors,
				"textures/maps/split_corridors/background.png"
			},
			TextureAssetDefinition{
				TextureId::BackgroundBrokenDistrict,
				"textures/maps/broken_district/background.png"
			},
			TextureAssetDefinition{
				TextureId::ObstacleCrossfire,
				"textures/maps/crossfire/obstacle.png"
			},
			TextureAssetDefinition{
				TextureId::ObstacleSplitCorridors,
				"textures/maps/split_corridors/obstacle.png"
			},
			TextureAssetDefinition{
				TextureId::ObstacleBrokenDistrict,
				"textures/maps/broken_district/obstacle.png"
			},
			TextureAssetDefinition{
				TextureId::EnemyChaser,
				"textures/entities/enemy_chaser.png"
			},
			TextureAssetDefinition{
				TextureId::EnemyShooter,
				"textures/entities/enemy_shooter.png"
			},
			TextureAssetDefinition{
				TextureId::Player,
				"textures/entities/player.png"
			},
			TextureAssetDefinition{
				TextureId::PlayerProjectile,
				"textures/projectiles/player.png"
			},
			TextureAssetDefinition{
				TextureId::EnemyProjectile,
				"textures/projectiles/enemy.png"
			}
		};

		return definitions;
	}

	const MapVisualDefinition& mapVisualFor(
		MapId mapId) noexcept {
		static const MapVisualDefinition legacy{
			MapId::Legacy,
			std::nullopt,
			std::nullopt,
			RgbColor{ 18u, 18u, 18u },
			RgbColor{ 100u, 100u, 120u }
		};
		static const MapVisualDefinition crossfire{
			MapId::Crossfire,
			TextureId::BackgroundCrossfire,
			TextureId::ObstacleCrossfire,
			RgbColor{ 4u, 17u, 30u },
			RgbColor{ 65u, 190u, 230u }
		};
		static const MapVisualDefinition splitCorridors{
			MapId::SplitCorridors,
			TextureId::BackgroundSplitCorridors,
			TextureId::ObstacleSplitCorridors,
			RgbColor{ 20u, 8u, 36u },
			RgbColor{ 190u, 75u, 245u }
		};
		static const MapVisualDefinition brokenDistrict{
			MapId::BrokenDistrict,
			TextureId::BackgroundBrokenDistrict,
			TextureId::ObstacleBrokenDistrict,
			RgbColor{ 31u, 18u, 7u },
			RgbColor{ 245u, 160u, 55u }
		};

		switch (mapId) {
		case MapId::Crossfire:
			return crossfire;
		case MapId::SplitCorridors:
			return splitCorridors;
		case MapId::BrokenDistrict:
			return brokenDistrict;
		case MapId::Legacy:
		default:
			return legacy;
		}
	}

	AssetLoadReport loadGameAssets(
		SDL_Renderer* renderer,
		AssetManager& assetManager) {
		AssetLoadReport report{};

		for (const TextureAssetDefinition& definition :
			textureAssetDefinitions()) {
			const std::string relativePath{
				definition.relativePath
			};
			const std::string absolutePath =
				assetFilePath(relativePath);

			if (absolutePath.empty()) {
				report.issues.push_back(
					AssetLoadIssue{
						relativePath,
						"SDL_GetBasePath failed"
					}
				);
				continue;
			}

			if (assetManager.loadTexture(
				renderer,
				definition.id,
				absolutePath)) {
				++report.loadedTextureCount;
				continue;
			}

			report.issues.push_back(
				AssetLoadIssue{
					relativePath,
					IMG_GetError()
				}
			);
		}

		return report;
	}

}//namespace neon::sdl
