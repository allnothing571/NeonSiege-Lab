#include "sdl/SdlPaths.h"

#include <SDL.h>

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

}//namespace neon::sdl