#include "sdl/AssetManager.h"

#include <SDL.h>
#include <SDL_image.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace {

	SDL_Rect visibleBounds(
		SDL_Surface* surface,
		Uint8 minimumVisibleAlpha) {
		SDL_Rect result{
			0,
			0,
			surface->w,
			surface->h
		};

		if (surface->format->BytesPerPixel != 4) {
			return result;
		}

		if (SDL_MUSTLOCK(surface) &&
			SDL_LockSurface(surface) != 0) {
			return result;
		}

		int minimumX = surface->w;
		int minimumY = surface->h;
		int maximumX = -1;
		int maximumY = -1;

		for (int y = 0; y < surface->h; ++y) {
			const auto* row = static_cast<const std::uint8_t*>(
				surface->pixels
			) + y * surface->pitch;

			for (int x = 0; x < surface->w; ++x) {
				std::uint32_t pixel = 0u;
				std::memcpy(
					&pixel,
					row + x * 4,
					sizeof(pixel)
				);

				Uint8 red = 0;
				Uint8 green = 0;
				Uint8 blue = 0;
				Uint8 alpha = 0;
				SDL_GetRGBA(
					pixel,
					surface->format,
					&red,
					&green,
					&blue,
					&alpha
				);

				if (alpha < minimumVisibleAlpha) {
					continue;
				}

				minimumX = std::min(minimumX, x);
				minimumY = std::min(minimumY, y);
				maximumX = std::max(maximumX, x);
				maximumY = std::max(maximumY, y);
			}
		}

		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}

		if (maximumX < minimumX || maximumY < minimumY) {
			return result;
		}

		result.x = minimumX;
		result.y = minimumY;
		result.w = maximumX - minimumX + 1;
		result.h = maximumY - minimumY + 1;
		return result;
	}

}//namespace

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

	bool AssetManager::loadTexture(
		SDL_Renderer* renderer,
		TextureId id,
		const std::string& path) {
		SDL_Surface* loadedSurface =
			IMG_Load(path.c_str());
		if (loadedSurface == nullptr) {
			return false;
		}

		SDL_Surface* convertedSurface =
			SDL_ConvertSurfaceFormat(
				loadedSurface,
				SDL_PIXELFORMAT_RGBA32,
				0
			);
		SDL_FreeSurface(loadedSurface);

		if (convertedSurface == nullptr) {
			return false;
		}

		const SDL_Rect sourceBounds =
			visibleBounds(
				convertedSurface,
				id == TextureId::ObstacleCrossfire ||
				id == TextureId::ObstacleSplitCorridors ||
				id == TextureId::ObstacleBrokenDistrict
				? 24u
				: 1u
			);
		SDL_Surface* textureSurface = convertedSurface;

		if (sourceBounds.x != 0 ||
			sourceBounds.y != 0 ||
			sourceBounds.w != convertedSurface->w ||
			sourceBounds.h != convertedSurface->h) {
			textureSurface = SDL_CreateRGBSurfaceWithFormat(
				0,
				sourceBounds.w,
				sourceBounds.h,
				32,
				SDL_PIXELFORMAT_RGBA32
			);

			if (textureSurface == nullptr ||
				SDL_BlitSurface(
					convertedSurface,
					&sourceBounds,
					textureSurface,
					nullptr
				) != 0) {
				if (textureSurface != nullptr) {
					SDL_FreeSurface(textureSurface);
				}
				SDL_FreeSurface(convertedSurface);
				return false;
			}
		}

		SDL_Texture* loadedTexture =
			SDL_CreateTextureFromSurface(
				renderer,
				textureSurface
			);

		if (textureSurface != convertedSurface) {
			SDL_FreeSurface(textureSurface);
		}
		SDL_FreeSurface(convertedSurface);

		if (loadedTexture == nullptr) {
			return false;
		}

		SDL_SetTextureBlendMode(
			loadedTexture,
			SDL_BLENDMODE_BLEND
		);
		SDL_SetTextureScaleMode(
			loadedTexture,
			SDL_ScaleModeNearest
		);
		storeTexture(id, loadedTexture);
		return true;
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
