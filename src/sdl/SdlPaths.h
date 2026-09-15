#pragma once

#include <string>

namespace neon::sdl {

	std::string preferenceFilePath(const std::string& fileName);
	std::string assetFilePath(const std::string& relativePath);

}//namespace neon::sdl
