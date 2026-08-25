#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <string_view>
#include <cmath>
#include <algorithm>
#include <vector>

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

		x_ = std::clamp(x_, 0.0f, windowHeight - size_);
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
};

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

	std::vector<Bullet> bullets;

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

			for (Bullet& bullet : bullets) {
				bullet.update(static_cast<float>(fixedDt));
			}

			bullets.erase(
				std::remove_if(
					bullets.begin(),
					bullets.end(),
					[](const Bullet& bullet) {
						return bullet.isOutside(960.0f, 540.0f);
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
