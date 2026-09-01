#pragma once

#include <array>
#include <cstddef>

struct SDL_Texture;

namespace neon::sdl {

	enum class TextureId : std::size_t {
		Background,
		Obstacle,
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