#include "SaveData.h"

#include <fstream>

int SaveData::loadHighScore(const std::string& path) const {
	std::ifstream input(path);
	int highScore = 0;

	if (!input || !(input >> highScore) || highScore < 0) {
		return 0;
	}

	return highScore;
}

bool SaveData::saveHighScore(
	const std::string& path,
	int highScore) const {
	std::ofstream output(path);

	if (!output) {
		return false;
	}

	output << highScore << '\n';
	return static_cast<bool>(output);
}