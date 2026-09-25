#include <set>
#include <string>

#include "core/MapDefinition.h"
#include "sdl/SdlAssetCatalog.h"

int main() {
	using neon::MapId;
	using neon::sdl::TextureId;

	const auto& definitions =
		neon::sdl::textureAssetDefinitions();
	std::set<std::string> paths{};

	for (const auto& definition : definitions) {
		paths.emplace(definition.relativePath);
	}

	const auto& crossfire =
		neon::sdl::mapVisualFor(MapId::Crossfire);
	const auto& splitCorridors =
		neon::sdl::mapVisualFor(MapId::SplitCorridors);
	const auto& brokenDistrict =
		neon::sdl::mapVisualFor(MapId::BrokenDistrict);
	const auto& legacy =
		neon::sdl::mapVisualFor(MapId::Legacy);

	const bool manifestPassed =
		definitions.size() == 11u &&
		paths.size() == definitions.size();
	const bool mapTexturesPassed =
		crossfire.backgroundTexture ==
			TextureId::BackgroundCrossfire &&
		crossfire.obstacleTexture ==
			TextureId::ObstacleCrossfire &&
		splitCorridors.backgroundTexture ==
			TextureId::BackgroundSplitCorridors &&
		splitCorridors.obstacleTexture ==
			TextureId::ObstacleSplitCorridors &&
		brokenDistrict.backgroundTexture ==
			TextureId::BackgroundBrokenDistrict &&
		brokenDistrict.obstacleTexture ==
			TextureId::ObstacleBrokenDistrict;
	const bool legacyFallbackPassed =
		!legacy.backgroundTexture.has_value() &&
		!legacy.obstacleTexture.has_value();

	return manifestPassed &&
		mapTexturesPassed &&
		legacyFallbackPassed
		? 0
		: 1;
}
