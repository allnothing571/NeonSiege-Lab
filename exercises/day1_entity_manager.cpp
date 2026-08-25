#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

class Entity {
public:
	Entity(const std::string& name, int hp)
		:name_(name), hp_(hp)
	{
	}

	void takeDamage(int damage) {
		if (damage <= 0) {
			return;
		}

		hp_ -= damage;

		if (hp_ < 0) {
			hp_ = 0;
		}
	}

	bool isAlive() const {
		return hp_ > 0;
	}

	const std::string& name() const {
		return name_;
	}

	int hp() const {
		return hp_;
	}

private:
	std::string name_;
	int hp_ = 0;
};

int main() {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
#endif

	std::vector<Entity> entities;

	entities.emplace_back("汪鸡", 9999);
	entities.emplace_back("小付", 9999);
	entities.emplace_back("鳄鱼", 1);

	for (Entity& entity : entities) {
		entity.takeDamage(10);
	}

	auto newEnd = std::remove_if(
		entities.begin(),
		entities.end(),
		[](const Entity& entity) {
			return !entity.isAlive();
		});

	entities.erase(newEnd, entities.end());

	std::ofstream output("day1_entities.txt");
	if (!output) {
		std::cerr << "无法创建 day1_entities.txt\n";
		return 1;
	}

	for (const Entity& entity : entities) {
		std::cout << entity.name() << ' ' << entity.hp() << '\n';
		output << entity.name() << ' ' << entity.hp() << '\n';
	}

	return 0;
}

