#include "sdl/AssetManager.h"

int main() {
	neon::sdl::AssetManager assets;

	const bool missingPlayerPassed =
		!assets.hasTexture(
			neon::sdl::TextureId::Player
		) &&
		assets.texture(
			neon::sdl::TextureId::Player
		) == nullptr;

	const bool missingEnemyPassed =
		!assets.hasTexture(
			neon::sdl::TextureId::EnemyChaser
		) &&
		assets.texture(
			neon::sdl::TextureId::EnemyChaser
		) == nullptr;

	return missingPlayerPassed &&
		missingEnemyPassed
		? 0
		: 1;
}