#pragma once

#include <array>
#include <cstddef>
#include <string>

struct SDL_Renderer;
struct SDL_Texture;

namespace neon::sdl {

	enum class TextureId : std::size_t {
		BackgroundCrossfire,
		BackgroundSplitCorridors,
		BackgroundBrokenDistrict,
		ObstacleCrossfire,
		ObstacleSplitCorridors,
		ObstacleBrokenDistrict,
		EnemyChaser,
		EnemyShooter,
		Player,
		PlayerProjectile,
		EnemyProjectile,
		Count
	};

	class AssetManager {
	public:
		AssetManager() = default;
		~AssetManager();

		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(
			const AssetManager&) = delete;

		void storeTexture(
			TextureId id,
			SDL_Texture* texture
		);

		bool loadTexture(
			SDL_Renderer* renderer,
			TextureId id,
			const std::string& path
		);

		SDL_Texture* texture(
			TextureId id
		) const;

		bool hasTexture(
			TextureId id
		) const;

		void clear();

	private:
		static std::size_t toIndex(
			TextureId id
		);

		std::array<
			SDL_Texture*,
			static_cast<std::size_t>(
				TextureId::Count
				)
		> textures_{};
	};

}//namespace neon::sdl
