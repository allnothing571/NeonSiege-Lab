#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "core/MapDefinition.h"
#include "sdl/AssetManager.h"

struct SDL_Renderer;

namespace neon::sdl {

	struct RgbColor {
		std::uint8_t red = 0u;
		std::uint8_t green = 0u;
		std::uint8_t blue = 0u;
	};

	struct TextureAssetDefinition {
		TextureId id = TextureId::Player;
		std::string_view relativePath{};
	};

	struct MapVisualDefinition {
		MapId mapId = MapId::Legacy;
		std::optional<TextureId> backgroundTexture{};
		std::optional<TextureId> obstacleTexture{};
		RgbColor backgroundFallback{};
		RgbColor obstacleFallback{};
	};

	struct AssetLoadIssue {
		std::string relativePath{};
		std::string message{};
	};

	struct AssetLoadReport {
		int loadedTextureCount = 0;
		std::vector<AssetLoadIssue> issues{};
	};

	const std::array<TextureAssetDefinition, 11>&
	textureAssetDefinitions() noexcept;

	const MapVisualDefinition& mapVisualFor(
		MapId mapId
	) noexcept;

	AssetLoadReport loadGameAssets(
		SDL_Renderer* renderer,
		AssetManager& assetManager
	);

}//namespace neon::sdl
