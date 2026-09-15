#include "sdl/SdlPaths.h"

#include <SDL.h>

#include <filesystem>

namespace neon::sdl {

	std::string preferenceFilePath(const std::string& fileName) {
		char* preferencePath =
			SDL_GetPrefPath("WangJiyi", "NeonSiege");

		if (preferencePath == nullptr) {
			return{};
		}

		const std::string result =
			std::string(preferencePath) + fileName;

		SDL_free(preferencePath);
		return result;
	}

	std::string assetFilePath(const std::string& relativePath) {
		char* basePath = SDL_GetBasePath();

		if (basePath == nullptr) {
			return{};
		}

		const std::filesystem::path result =
			std::filesystem::path(basePath) /
			"assets" /
			relativePath;

		SDL_free(basePath);
		return result.lexically_normal().string();
	}

}//namespace neon::sdl
