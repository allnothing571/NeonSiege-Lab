#include "sdl/AssetManager.h"

#include <SDL.h>

namespace neon::sdl {

	AssetManager::~AssetManager() {
		clear();
	}

	void AssetManager::storeTexture(
		TextureId id,
		SDL_Texture* texture) {

		SDL_Texture*& currentTexture =
			textures_[toIndex(id)];

		if (currentTexture == texture) {
			return;
		}

		if (currentTexture != nullptr) {
			SDL_DestroyTexture(currentTexture);
		}

		currentTexture = texture;
	}

	SDL_Texture* AssetManager::texture(
		TextureId id) const {

		return textures_[toIndex(id)];
	}

	bool AssetManager::hasTexture(
		TextureId id) const {
		return texture(id) != nullptr;
	}

	void AssetManager::clear() {
		for (SDL_Texture*& texture : textures_) {
			if (texture != nullptr) {
				SDL_DestroyTexture(texture);
				texture = nullptr;
			}
		}
	}

	std::size_t AssetManager::toIndex(
		TextureId id) {

		return static_cast<std::size_t>(id);
	}

}//namespace neon::sdl