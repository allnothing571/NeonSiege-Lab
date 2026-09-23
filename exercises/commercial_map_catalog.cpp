#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <set>
#include <string_view>
#include <vector>

#include "core/Collision.h"
#include "core/GameplayConfig.h"
#include "core/MapCatalog.h"
#include "core/Simulation.h"

namespace {

	bool finiteRect(
		const neon::Rect& rectangle) {
		return std::isfinite(rectangle.x) &&
			std::isfinite(rectangle.y) &&
			std::isfinite(rectangle.w) &&
			std::isfinite(rectangle.h);
	}

	bool inside(
		const neon::Rect& candidate,
		const neon::Rect& worldBounds) {
		return candidate.x >= worldBounds.x &&
			candidate.y >= worldBounds.y &&
			candidate.x + candidate.w <=
				worldBounds.x + worldBounds.w &&
			candidate.y + candidate.h <=
				worldBounds.y + worldBounds.h;
	}

	bool clearOfObstacles(
		const neon::Rect& area,
		const std::vector<neon::Rect>& obstacles) {
		return std::none_of(
			obstacles.begin(),
			obstacles.end(),
			[&area](const neon::Rect& obstacle) {
				return neon::intersects(area, obstacle);
			}
		);
	}

	bool hasFourOpenRoutes(
		const neon::MapDefinition& map,
		const neon::Rect& worldBounds,
		float playerSize) {
		const neon::Rect playerBounds{
			map.playerStartPosition.x,
			map.playerStartPosition.y,
			playerSize,
			playerSize
		};

		const float centerX =
			playerBounds.x + playerBounds.w * 0.5f;
		const float centerY =
			playerBounds.y + playerBounds.h * 0.5f;
		const float halfClearance = playerSize * 0.5f;

		const std::vector<neon::Rect> routes{
			neon::Rect{
				worldBounds.x,
				centerY - halfClearance,
				centerX - worldBounds.x,
				playerSize
			},
			neon::Rect{
				centerX,
				centerY - halfClearance,
				worldBounds.x + worldBounds.w - centerX,
				playerSize
			},
			neon::Rect{
				centerX - halfClearance,
				worldBounds.y,
				playerSize,
				centerY - worldBounds.y
			},
			neon::Rect{
				centerX - halfClearance,
				centerY,
				playerSize,
				worldBounds.y + worldBounds.h - centerY
			}
		};

		return std::all_of(
			routes.begin(),
			routes.end(),
			[&map](const neon::Rect& route) {
				return clearOfObstacles(
					route,
					map.obstacleBounds
				);
			}
		);
	}

	bool mapIsValid(
		const neon::MapDefinition& map,
		const neon::GameplayConfig& config) {
		if (map.key.empty() ||
			map.obstacleBounds.empty()) {
			return false;
		}

		const neon::Rect playerBounds{
			map.playerStartPosition.x,
			map.playerStartPosition.y,
			config.playerSize,
			config.playerSize
		};

		if (!finiteRect(playerBounds) ||
			!inside(playerBounds, config.worldBounds) ||
			!clearOfObstacles(
				playerBounds,
				map.obstacleBounds)) {
			return false;
		}

		for (std::size_t first = 0;
			first < map.obstacleBounds.size();
			++first) {
			const neon::Rect& obstacle =
				map.obstacleBounds[first];

			if (!finiteRect(obstacle) ||
				obstacle.w <= 0.0f ||
				obstacle.h <= 0.0f ||
				!inside(obstacle, config.worldBounds)) {
				return false;
			}

			for (std::size_t second = first + 1;
				second < map.obstacleBounds.size();
				++second) {
				if (neon::intersects(
					obstacle,
					map.obstacleBounds[second])) {
					return false;
				}
			}
		}

		return hasFourOpenRoutes(
			map,
			config.worldBounds,
			config.playerSize
		);
	}

	bool catalogPassed() {
		const neon::GameplayConfig config{};
		const std::vector<neon::MapDefinition>& maps =
			neon::presetMaps();

		if (maps.size() != 3u) {
			return false;
		}

		std::set<std::uint32_t> ids;
		std::set<std::string_view> keys;

		for (const neon::MapDefinition& map : maps) {
			ids.insert(static_cast<std::uint32_t>(map.id));
			keys.insert(map.key);

			if (!mapIsValid(map, config) ||
				neon::findPresetMap(map.id) != &map) {
				return false;
			}
		}

		return ids.size() == maps.size() &&
			keys.size() == maps.size() &&
			neon::findPresetMap(
				static_cast<neon::MapId>(999u)) == nullptr;
	}

	bool randomSelectionPassed() {
		std::mt19937 randomEngine{ 1337u };
		std::optional<neon::MapId> previousMap;
		std::set<std::uint32_t> selectedIds;

		for (int iteration = 0;
			iteration < 64;
			++iteration) {
			const neon::MapId selected =
				neon::chooseRandomPresetMap(
					randomEngine,
					previousMap
				);

			if (neon::findPresetMap(selected) == nullptr ||
				(previousMap.has_value() &&
					selected == *previousMap)) {
				return false;
			}

			selectedIds.insert(
				static_cast<std::uint32_t>(selected)
			);
			previousMap = selected;
		}

		return selectedIds.size() ==
			neon::presetMaps().size();
	}

	bool simulationIntegrationPassed() {
		neon::GameplayConfig config{};
		neon::Simulation simulation(config);
		const neon::MapDefinition& map =
			neon::presetMaps().back();

		if (!simulation.reset(map.id)) {
			return false;
		}

		const neon::GameSnapshot selected =
			simulation.snapshot();

		if (selected.mapId != map.id ||
			selected.player.bounds.x !=
				map.playerStartPosition.x ||
			selected.player.bounds.y !=
				map.playerStartPosition.y ||
			selected.obstacles.size() !=
				map.obstacleBounds.size()) {
			return false;
		}

		for (std::size_t index = 0;
			index < map.obstacleBounds.size();
			++index) {
			const neon::Rect actual =
				selected.obstacles[index].bounds;
			const neon::Rect expected =
				map.obstacleBounds[index];

			if (actual.x != expected.x ||
				actual.y != expected.y ||
				actual.w != expected.w ||
				actual.h != expected.h) {
				return false;
			}
		}

		simulation.reset();
		const neon::GameSnapshot restarted =
			simulation.snapshot();

		if (restarted.mapId != map.id ||
			restarted.obstacles.size() !=
				map.obstacleBounds.size()) {
			return false;
		}

		const neon::MapId invalidMap =
			static_cast<neon::MapId>(999u);
		return !simulation.reset(invalidMap) &&
			simulation.snapshot().mapId == map.id;
	}

}//namespace

int main() {
	return catalogPassed() &&
		randomSelectionPassed() &&
		simulationIntegrationPassed()
		? 0
		: 1;
}
