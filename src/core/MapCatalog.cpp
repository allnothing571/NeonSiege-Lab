#include "core/MapCatalog.h"

#include <algorithm>
#include <iterator>

namespace neon {

	const std::vector<MapDefinition>& presetMaps() {
		static const std::vector<MapDefinition> maps{
			MapDefinition{
				MapId::Crossfire,
				"crossfire",
				Vec2{ 616.0f, 336.0f },
				std::vector<Rect>{
					Rect{ 220.0f, 120.0f, 180.0f, 32.0f },
					Rect{ 880.0f, 120.0f, 180.0f, 32.0f },
					Rect{ 120.0f, 288.0f, 120.0f, 32.0f },
					Rect{ 1040.0f, 288.0f, 120.0f, 32.0f },
					Rect{ 220.0f, 568.0f, 180.0f, 32.0f },
					Rect{ 880.0f, 568.0f, 180.0f, 32.0f }
				}
			},
			MapDefinition{
				MapId::SplitCorridors,
				"split_corridors",
				Vec2{ 616.0f, 336.0f },
				std::vector<Rect>{
					Rect{ 360.0f, 80.0f, 42.0f, 220.0f },
					Rect{ 878.0f, 80.0f, 42.0f, 220.0f },
					Rect{ 360.0f, 420.0f, 42.0f, 220.0f },
					Rect{ 878.0f, 420.0f, 42.0f, 220.0f }
				}
			},
			MapDefinition{
				MapId::BrokenDistrict,
				"broken_district",
				Vec2{ 616.0f, 336.0f },
				std::vector<Rect>{
					Rect{ 100.0f, 80.0f, 210.0f, 38.0f },
					Rect{ 720.0f, 120.0f, 44.0f, 150.0f },
					Rect{ 1010.0f, 180.0f, 160.0f, 38.0f },
					Rect{ 150.0f, 400.0f, 130.0f, 38.0f },
					Rect{ 410.0f, 510.0f, 170.0f, 38.0f },
					Rect{ 800.0f, 390.0f, 48.0f, 150.0f },
					Rect{ 1030.0f, 560.0f, 130.0f, 38.0f }
				}
			}
		};

		return maps;
	}

	const MapDefinition* findPresetMap(
		MapId id) noexcept {
		const std::vector<MapDefinition>& maps =
			presetMaps();
		const auto match = std::find_if(
			maps.begin(),
			maps.end(),
			[id](const MapDefinition& map) {
				return map.id == id;
			}
		);

		return match != maps.end()
			? &*match
			: nullptr;
	}

	MapId chooseRandomPresetMap(
		std::mt19937& randomEngine,
		std::optional<MapId> previousMap) {
		const std::vector<MapDefinition>& maps =
			presetMaps();

		if (maps.empty()) {
			return MapId::Legacy;
		}

		const auto previous = previousMap.has_value()
			? std::find_if(
				maps.begin(),
				maps.end(),
				[previousMap](const MapDefinition& map) {
					return map.id == *previousMap;
				}
			)
			: maps.end();

		if (maps.size() == 1u ||
			previous == maps.end()) {
			std::uniform_int_distribution<std::size_t>
				distribution(0u, maps.size() - 1u);
			return maps[distribution(randomEngine)].id;
		}

		const std::size_t previousIndex =
			static_cast<std::size_t>(
				std::distance(maps.begin(), previous)
			);
		std::uniform_int_distribution<std::size_t>
			distribution(0u, maps.size() - 2u);
		std::size_t selectedIndex =
			distribution(randomEngine);

		if (selectedIndex >= previousIndex) {
			++selectedIndex;
		}

		return maps[selectedIndex].id;
	}

}//namespace neon
