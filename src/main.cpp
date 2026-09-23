#define SDL_MAIN_HANDLED

#include <string_view>

#include "GameApplication.h"

int main(int argc, char* argv[]) {
	const bool smokeTest =
		argc > 1 &&
		std::string_view(argv[1]) == "--smoke-test";
	return runNeonSiege(smokeTest);
}
