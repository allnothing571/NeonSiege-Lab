#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <string_view>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

class Player {
public:
	Player(float x, float y, float size, float speed)
		: x_(x), y_(y), size_(size), speed_(speed) {

	}

	void update(
		float directionX,
		float directionY,
		float dt,
		float windowWidth,
		float windowHeight) {

		if (y_ <= 0 && directionY < 0.0f) {
			directionY = 0.0f;
		}
		if (y_ >= windowHeight - size_ && directionY > 0.0f) {
			directionY = 0.0f;
		}
		if (x_ <= 0 && directionX < 0.0f) {
			directionX = 0.0f;
		}
		if (x_ >= windowWidth - size_ && directionX > 0.0f) {
			directionX = 0.0f;
		}

		const float directionLength =
			std::sqrt(directionX * directionX + directionY * directionY);

		if (directionLength > 0.0f) {
			directionX /= directionLength;
			directionY /= directionLength;
		}

		x_ += directionX * speed_ * dt;
		y_ += directionY * speed_ * dt;

		x_ = std::clamp(x_, 0.0f, windowWidth - size_);
		y_ = std::clamp(y_, 0.0f, windowHeight - size_);
	}

	void render(SDL_Renderer* renderer) const {
		SDL_FRect playerRect{
			x_,
			y_,
			size_,
			size_
		};

		SDL_SetRenderDrawColor(renderer, 0, 220, 255, 255);
		SDL_RenderFillRectF(renderer, &playerRect);
	}

	float centerX() const {
		return x_ + size_ / 2.0f;
	}

	float centerY() const {
		return y_ + size_ / 2.0f;
	}

private:
	float x_;
	float y_;
	float size_;
	float speed_;
};

class Bullet {
public:
	Bullet(
		float x,
		float y,
		float directionX,
		float directionY,
		float size,
		float speed)
		: x_(x),
		y_(y),
		directionX_(directionX),
		directionY_(directionY),
		size_(size),
		speed_(speed) {
	}

	void update(float dt) {
		x_ += directionX_ * speed_ * dt;
		y_ += directionY_ * speed_ * dt;
	}

	bool isOutside(float windowWidth,
		float windowHeight)const {

		return
			x_ + size_ < 0.0f ||
			x_ > windowWidth ||
			y_ + size_ < 0.0f ||
			y_ > windowHeight;
	}

	SDL_FRect bounds() const {
		return SDL_FRect{ x_,y_,size_,size_ };
	}

	void consume() {
		consumed_ = true;
	}

	bool isConsumed() const {
		return consumed_;
	}

	void render(SDL_Renderer* renderer) const {
		SDL_FRect bulletRect{
			x_,
			y_,
			size_,
			size_
		};

		SDL_SetRenderDrawColor(renderer, 255, 220, 80, 255);
		SDL_RenderFillRectF(renderer, &bulletRect);
	}

private:
	float x_;
	float y_;
	float directionX_;
	float directionY_;
	float size_;
	float speed_;
	bool consumed_ = false;
};

class Enemy {
public:
	Enemy(float x, float y, float size, float speed, int hp)
		:x_(x), y_(y), size_(size), speed_(speed), hp_(hp) {
	}

	void update(float targetX, float targetY, float dt);

	void takeDamage(int damage);

	bool isAlive() const;

	void render(SDL_Renderer* renderer) const;

	SDL_FRect bounds() const {
		return SDL_FRect{ x_,y_,size_,size_ };
	}

private:
	float x_;
	float y_;
	float size_;
	float speed_;
	int hp_;
};

void Enemy::update(float targetX, float targetY, float dt) {
	const float enemyCenterX = x_ + size_ / 2.0f;
	const float enemyCenterY = y_ + size_ / 2.0f;

	float directionX = targetX - enemyCenterX;
	float directionY = targetY - enemyCenterY;

	const float directionLength =
		std::sqrt(directionX * directionX + directionY * directionY);

	if (directionLength > 0.0f) {
		directionX /= directionLength;
		directionY /= directionLength;
	}

	x_ += directionX * speed_ * dt;
	y_ += directionY * speed_ * dt;
}

void Enemy::takeDamage(int damage) {
	if (damage <= 0) {
		return;
	}

	hp_ -= damage;

	if (hp_ < 0) {
		hp_ = 0;
	}
}

bool Enemy::isAlive() const {
	return hp_ > 0;
}

void Enemy::render(SDL_Renderer* renderer) const {
	SDL_FRect enemyRect{
		x_,
		y_,
		size_,
		size_
	};

	SDL_SetRenderDrawColor(renderer, 220, 60, 70, 255);
	SDL_RenderFillRectF(renderer, &enemyRect);
}

bool intersects(const SDL_FRect first, const SDL_FRect second) {
	return
		first.x < second.x + second.w &&
		first.x + first.w > second.x &&
		first.y < second.y + second.h &&
		first.y + first.h > second.y;
}

int main(int argc, char* argv[]) {
	const bool smokeTest = argc > 1 && std::string_view(argv[1]) == "--smoke-test";

	SDL_SetMainReady();

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Neon Siege - Day 3",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		960,
		540,
		smokeTest ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);

	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == nullptr) {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	}

	if (renderer == nullptr) {
		std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	const double frequency =
		static_cast<double>(SDL_GetPerformanceFrequency());
	Uint64 previousCounter = SDL_GetPerformanceCounter();

	Player player(456.0f, 246.0f, 48.0f, 240.0f);

	//创建子弹
	std::vector<Bullet> bullets;

	//创建敌人
	std::vector<Enemy> enemies;
	enemies.emplace_back(100.0f, 100.0f, 32.0f, 80.0f, 30);
	enemies.emplace_back(800.0f, 100.0f, 32.0f, 70.0f, 30);
	enemies.emplace_back(100.0f, 400.0f, 32.0f, 90.0f, 30);

	int score = 0;

	constexpr double fixedDt = 1.0 / 60.0;
	double accumulator = 0.0;

	bool running = true;
	while (running) {
		const Uint64 currentCounter = SDL_GetPerformanceCounter();
		constexpr double maxFrameTime = 0.25;

		const double rawFrameTime =
			(currentCounter - previousCounter) / frequency;

		const double frameTime =
			std::min(rawFrameTime, maxFrameTime);
		previousCounter = currentCounter;
		accumulator += frameTime;

		bool fireRequested = false;

		SDL_Event event{};
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				running = false;
			}
			if (event.type == SDL_MOUSEBUTTONDOWN &&
				event.button.button == SDL_BUTTON_LEFT) {
				fireRequested = true;
			}
		}

		while (accumulator >= fixedDt) {
			const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);

			float directionX = 0.0f;
			float directionY = 0.0f;

			if (keyboardState[SDL_SCANCODE_W]) {
				directionY -= 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_A]) {
				directionX -= 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_S]) {
				directionY += 1.0f;
			}
			if (keyboardState[SDL_SCANCODE_D]) {
				directionX += 1.0f;
			}

			player.update(
				directionX,
				directionY,
				static_cast<float>(fixedDt),
				960.0f,
				540.0f);

			for (Enemy& enemy : enemies) {
				enemy.update(
					player.centerX(),
					player.centerY(),
					static_cast<float>(fixedDt)
				);
			}

			for (Bullet& bullet : bullets) {
				bullet.update(static_cast<float>(fixedDt));
			}

			for (Bullet& bullet : bullets) {
				for (Enemy& enemy : enemies) {
					if (!enemy.isAlive()) {
						continue;
					}

					if (intersects(bullet.bounds(), enemy.bounds())) {
						enemy.takeDamage(10);
						bullet.consume();

						if (!enemy.isAlive()) {
							score += 100;
						}

						break;
					}
				}
			}

			enemies.erase(
				std::remove_if(
					enemies.begin(),
					enemies.end(),
					[](const Enemy& enemy) {
						return !enemy.isAlive();
					}),
				enemies.end());

			bullets.erase(
				std::remove_if(
					bullets.begin(),
					bullets.end(),
					[](const Bullet& bullet) {
						return bullet.isConsumed() ||
							bullet.isOutside(960.0f, 540.0f);
					}),
				bullets.end());

			accumulator -= fixedDt;
		}

		int mouseX = 0;
		int mouseY = 0;
		SDL_GetMouseState(&mouseX, &mouseY);

		const float playerCenterX = player.centerX();
		const float playerCenterY = player.centerY();

		float aimX = static_cast<float>(mouseX) - playerCenterX;
		float aimY = static_cast<float>(mouseY) - playerCenterY;

		const float aimLength =
			std::sqrt(aimX * aimX + aimY * aimY);

		if (aimLength > 0.0f) {
			aimX /= aimLength;
			aimY /= aimLength;
		}

		if (fireRequested && aimLength > 0.0f) {
			bullets.emplace_back(
				playerCenterX,
				playerCenterY,
				aimX,
				aimY,
				8.0f,
				600.0f);
		}

		SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
		SDL_RenderClear(renderer);

		//绘制敌人
		for (const Enemy& enemy : enemies) {
			enemy.render(renderer);
		}

		//绘制玩家
		player.render(renderer);

		//绘制子弹
		for (const Bullet& bullet : bullets) {
			bullet.render(renderer);
		}

		//朝向线绘制
		constexpr float aimLineLength = 60.0f;

		SDL_SetRenderDrawColor(renderer, 255, 80, 160, 255);
		SDL_RenderDrawLineF(
			renderer,
			playerCenterX,
			playerCenterY,
			playerCenterX + aimX * aimLineLength,
			playerCenterY + aimY * aimLineLength
		);

		//计分板绘制
		const std::string title =
			"Neon Siege - Day 4 | Score: " + std::to_string(score);

		SDL_SetWindowTitle(window, title.c_str());

		SDL_RenderPresent(renderer);

		if (smokeTest) {
			running = false;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
