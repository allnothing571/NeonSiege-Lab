#pragma once

#include <string>

class SaveData {
public:
	int loadHighScore(const std::string& path) const;

	bool saveHighScore(
		const std::string& path,
		int highScore) const;
};