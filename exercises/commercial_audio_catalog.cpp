#include "sdl/SdlAudioCatalog.h"

#include <set>
#include <string>

int main() {
	const auto& definitions =
		neon::sdl::soundAssetDefinitions();
	if (definitions.size() != neon::sdl::soundAssetCount ||
		definitions.size() != 14u) {
		return 1;
	}

	std::set<std::string> paths{};
	for (std::size_t index = 0;
		index < definitions.size();
		++index) {

		const auto& definition = definitions[index];
		if (static_cast<std::size_t>(definition.id) != index ||
			definition.relativePath.empty() ||
			definition.baseVolumePercent < 0 ||
			definition.baseVolumePercent > 100 ||
			!paths.insert(
				std::string(definition.relativePath)
			).second) {

			return 1;
		}

		if (&neon::sdl::soundAssetFor(definition.id) !=
			&definition) {
			return 1;
		}
	}

	return 0;
}
