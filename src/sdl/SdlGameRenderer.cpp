#include "sdl/SdlGameRenderer.h"

#include <SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include "core/GameSnapshot.h"
#include "sdl/AssetManager.h"
#include "sdl/SdlAssetCatalog.h"

namespace neon::sdl {

	namespace {

		struct CanvasSize {
			int width = 1280;
			int height = 720;
		};

		CanvasSize canvasSize(
			SDL_Renderer* renderer) {
			CanvasSize size{};
			int logicalWidth = 0;
			int logicalHeight = 0;
			SDL_RenderGetLogicalSize(
				renderer,
				&logicalWidth,
				&logicalHeight
			);
			if (logicalWidth > 0 && logicalHeight > 0) {
				size.width = logicalWidth;
				size.height = logicalHeight;
				return size;
			}

			SDL_GetRendererOutputSize(
				renderer,
				&size.width,
				&size.height
			);
			return size;
		}

		SDL_Rect centeredButtonRect(
			SDL_Renderer* renderer,
			int centerY,
			int width) {
			const CanvasSize size = canvasSize(renderer);
			return SDL_Rect{
				(size.width - width) / 2,
				centerY - 26,
				width,
				52
			};
		}

		std::array<SDL_Rect, 3> upgradeCardRects(
			SDL_Renderer* renderer) {
			const CanvasSize size = canvasSize(renderer);
			constexpr int gap = 24;
			const int availableWidth =
				std::max(720, size.width - 80);
			const int cardWidth =
				std::min(340, (availableWidth - gap * 2) / 3);
			const int totalWidth =
				cardWidth * 3 + gap * 2;
			const int startX =
				(size.width - totalWidth) / 2;
			const int cardHeight =
				std::min(360, std::max(300, size.height - 300));
			const int top =
				std::max(150, (size.height - cardHeight) / 2);

			return {
				SDL_Rect{ startX, top, cardWidth, cardHeight },
				SDL_Rect{
					startX + cardWidth + gap,
					top,
					cardWidth,
					cardHeight
				},
				SDL_Rect{
					startX + (cardWidth + gap) * 2,
					top,
					cardWidth,
					cardHeight
				}
			};
		}

		bool contains(
			const SDL_Rect& rectangle,
			int x,
			int y) {
			return x >= rectangle.x &&
				x < rectangle.x + rectangle.w &&
				y >= rectangle.y &&
				y < rectangle.y + rectangle.h;
		}

	}//namespace

	SdlGameRenderer::SdlGameRenderer(
		SDL_Renderer* renderer,
		const AssetManager& assetManager)
		: renderer_(renderer),
		assetManager_(assetManager),
		textRenderer_() {
	}

	void SdlGameRenderer::setLanguage(
		UiLanguage language) noexcept {
		language_ = language;
	}

	void SdlGameRenderer::setPauseMenuReturnEnabled(
		bool enabled) noexcept {
		pauseMenuReturnEnabled_ = enabled;
	}

	void SdlGameRenderer::setUpgradeSelection(
		int selectedOption) noexcept {
		selectedUpgradeOption_ =
			std::clamp(selectedOption, 0, 2);
	}

	void SdlGameRenderer::resetPresentationEffects() {
		presentationEffects_.reset();
		lastRenderedState_ = GameState::Playing;
	}

	bool SdlGameRenderer::chineseText() const noexcept {
		return language_ == UiLanguage::Chinese &&
			textRenderer_.supportsChinese();
	}

	void SdlGameRenderer::renderFrontendBackground() const {
		const CanvasSize size = canvasSize(renderer_);
		SDL_SetRenderDrawColor(renderer_, 7, 9, 18, 255);
		SDL_RenderClear(renderer_);

		SDL_SetRenderDrawColor(renderer_, 18, 32, 55, 255);
		for (int x = 0; x < size.width; x += 64) {
			SDL_RenderDrawLine(
				renderer_,
				x,
				0,
				x,
				size.height
			);
		}
		for (int y = 0; y < size.height; y += 64) {
			SDL_RenderDrawLine(
				renderer_,
				0,
				y,
				size.width,
				y
			);
		}
	}

	void SdlGameRenderer::renderFrontendButton(
		std::string_view text,
		int centerY,
		int width,
		bool selected) const {
		const CanvasSize size = canvasSize(renderer_);
		const SDL_Rect button =
			centeredButtonRect(renderer_, centerY, width);

		SDL_SetRenderDrawColor(
			renderer_,
			selected ? 42 : 18,
			selected ? 55 : 25,
			selected ? 82 : 42,
			245
		);
		SDL_RenderFillRect(renderer_, &button);
		SDL_SetRenderDrawColor(
			renderer_,
			selected ? 255 : 75,
			selected ? 80 : 120,
			selected ? 150 : 170,
			255
		);
		SDL_RenderDrawRect(renderer_, &button);

		const SDL_Color textColor = selected
			? SDL_Color{ 255, 230, 240, 255 }
			: SDL_Color{ 210, 220, 235, 255 };
		textRenderer_.drawCentered(
			renderer_,
			text,
			TextStyle::Body,
			size.width / 2,
			centerY,
			textColor
		);
	}

	void SdlGameRenderer::renderMainMenu(
		int selectedItem) const {
		const CanvasSize size = canvasSize(renderer_);
		renderFrontendBackground();
		const bool chinese = chineseText();

		textRenderer_.drawCentered(
			renderer_,
			chinese ? "霓虹防线" : "NEON SIEGE",
			TextStyle::Title,
			size.width / 2,
			120,
			SDL_Color{ 255, 75, 145, 255 }
		);
		textRenderer_.drawCentered(
			renderer_,
			chinese ? "守住防线，迎战十波敌人" :
				"HOLD THE LINE THROUGH TEN WAVES",
			TextStyle::Body,
			size.width / 2,
			178,
			SDL_Color{ 125, 205, 255, 255 }
		);

		const std::array<std::string_view, 4> chineseItems{
			"开始游戏", "游玩指南", "设置", "退出游戏"
		};
		const std::array<std::string_view, 4> englishItems{
			"START GAME", "HOW TO PLAY", "SETTINGS", "EXIT"
		};
		for (int index = 0; index < 4; ++index) {
			renderFrontendButton(
				chinese ? chineseItems[index] : englishItems[index],
				300 + index * 72,
				420,
				selectedItem == index
			);
		}

		textRenderer_.drawCentered(
			renderer_,
			chinese ? "方向键选择  Enter 确认" :
				"ARROW KEYS TO SELECT  ENTER TO CONFIRM",
			TextStyle::Body,
			size.width / 2,
			size.height - 38,
			SDL_Color{ 125, 135, 155, 255 }
		);
		SDL_RenderPresent(renderer_);
	}

	void SdlGameRenderer::renderHowToPlay(
		bool backHovered) const {
		const CanvasSize size = canvasSize(renderer_);
		renderFrontendBackground();
		const bool chinese = chineseText();

		textRenderer_.drawCentered(
			renderer_,
			chinese ? "游玩指南" : "HOW TO PLAY",
			TextStyle::Title,
			size.width / 2,
			105,
			SDL_Color{ 255, 90, 150, 255 }
		);

		const std::array<std::string_view, 5> chineseLines{
			"WASD 移动    鼠标瞄准    左键开火",
			"R 键换弹    Esc 暂停游戏",
			"利用障碍物躲避敌人和远程子弹",
			"清除全部敌人进入下一波，共十波",
			"第 2、4、6、8 波后必须选择一项升级"
		};
		const std::array<std::string_view, 5> englishLines{
			"WASD MOVE    MOUSE AIM    LEFT CLICK FIRE",
			"R RELOAD    ESC PAUSE",
			"USE COVER TO AVOID ENEMIES AND PROJECTILES",
			"CLEAR ALL ENEMIES TO ADVANCE THROUGH TEN WAVES",
			"CHOOSE AN UPGRADE AFTER WAVES 2, 4, 6 AND 8"
		};
		for (int index = 0; index < 5; ++index) {
			textRenderer_.drawCentered(
				renderer_,
				chinese ? chineseLines[index] : englishLines[index],
				TextStyle::Body,
				size.width / 2,
				205 + index * 54,
				SDL_Color{ 220, 225, 238, 255 }
			);
		}

		renderFrontendButton(
			chinese ? "[Esc / Enter] 返回主菜单" :
				"[ESC / ENTER] BACK TO MAIN MENU",
			size.height - 82,
			520,
			backHovered
		);
		SDL_RenderPresent(renderer_);
	}

	void SdlGameRenderer::renderSettings(
		const AppSettings& settings,
		int selectedItem) const {
		const CanvasSize size = canvasSize(renderer_);
		renderFrontendBackground();
		const bool chinese = chineseText();
		const WindowSizePreset windowSize =
			windowSizePresets()[std::clamp(
				settings.windowSizeIndex,
				0,
				static_cast<int>(windowSizePresets().size()) - 1
			)];

		textRenderer_.drawCentered(
			renderer_,
			chinese ? "设置" : "SETTINGS",
			TextStyle::Title,
			size.width / 2,
			90,
			SDL_Color{ 255, 90, 150, 255 }
		);

		const std::array<std::string, 5> items{
			chinese
				? "语言：" + std::string(
					settings.language == UiLanguage::Chinese
						? "简体中文" : "English")
				: "LANGUAGE: " + std::string(
					settings.language == UiLanguage::Chinese
						? "CHINESE" : "ENGLISH"),
			(chinese ? "窗口大小：" : "WINDOW SIZE: ") +
				std::to_string(windowSize.width) + " x " +
				std::to_string(windowSize.height),
			chinese
				? "窗口模式：" + std::string(
					settings.windowMode == WindowMode::Windowed
						? "窗口化" : "无边框全屏")
				: "WINDOW MODE: " + std::string(
					settings.windowMode == WindowMode::Windowed
						? "WINDOWED" : "BORDERLESS FULLSCREEN"),
			chinese ? "应用并返回" : "APPLY AND RETURN",
			chinese ? "取消" : "CANCEL"
		};

		for (int index = 0; index < 5; ++index) {
			renderFrontendButton(
				items[index],
				205 + index * 72,
				680,
				selectedItem == index
			);
		}

		textRenderer_.drawCentered(
			renderer_,
			chinese ? "左右方向键调整选项" :
				"USE LEFT AND RIGHT TO CHANGE VALUES",
			TextStyle::Body,
			size.width / 2,
			size.height - 40,
			SDL_Color{ 125, 135, 155, 255 }
		);
		SDL_RenderPresent(renderer_);
	}

	int SdlGameRenderer::mainMenuItemAt(
		int x,
		int y) const {
		for (int index = 0; index < 4; ++index) {
			if (contains(
				centeredButtonRect(
					renderer_,
					300 + index * 72,
					420
				),
				x,
				y)) {
				return index;
			}
		}
		return -1;
	}

	int SdlGameRenderer::settingsItemAt(
		int x,
		int y) const {
		for (int index = 0; index < 5; ++index) {
			if (contains(
				centeredButtonRect(
					renderer_,
					205 + index * 72,
					680
				),
				x,
				y)) {
				return index;
			}
		}
		return -1;
	}

	bool SdlGameRenderer::howToPlayBackAt(
		int x,
		int y) const {
		const CanvasSize size = canvasSize(renderer_);
		return contains(
			centeredButtonRect(
				renderer_,
				size.height - 82,
				520
			),
			x,
			y
		);
	}

	int SdlGameRenderer::upgradeOptionAt(
		int x,
		int y) const {
		const auto rectangles =
			upgradeCardRects(renderer_);
		for (int index = 0;
			index < static_cast<int>(rectangles.size());
			++index) {
			if (contains(rectangles[index], x, y)) {
				return index;
			}
		}
		return -1;
	}


	void SdlGameRenderer::renderHud(
		const GameSnapshot& snapshot) const {

		const CanvasSize size = canvasSize(renderer_);
		const int outputWidth = size.width;
		if (outputWidth <= 0 || size.height <= 0) {
			return;
		}

		const int barHeight = 48;
		SDL_BlendMode previousBlendMode =
			SDL_BLENDMODE_NONE;
		SDL_GetRenderDrawBlendMode(
			renderer_,
			&previousBlendMode
		);
		SDL_SetRenderDrawBlendMode(
			renderer_,
			SDL_BLENDMODE_BLEND
		);

		SDL_SetRenderDrawColor(
			renderer_,
			8,
			8,
			16,
			220
		);
		const SDL_Rect bar{
			0,
			0,
			outputWidth,
			barHeight
		};
		SDL_RenderFillRect(renderer_, &bar);

		const int maxHealth =
			std::max(1, snapshot.player.maxHealth);
		const int health =
			std::clamp(
				snapshot.player.health,
				0,
				maxHealth
			);
		const int healthWidth = 156;
		const SDL_Rect healthBackground{
			16,
			22,
			healthWidth,
			12
		};
		SDL_SetRenderDrawColor(
			renderer_,
			70,
			70,
			80,
			255
		);
		SDL_RenderFillRect(renderer_, &healthBackground);

		const SDL_Rect healthFill{
			16,
			22,
			healthWidth * health / maxHealth,
			12
		};
		SDL_SetRenderDrawColor(
			renderer_,
			health > maxHealth / 3 ? 60 : 240,
			health > maxHealth / 3 ? 220 : 70,
			90,
			255
		);
		SDL_RenderFillRect(renderer_, &healthFill);

		const bool useChinese = chineseText();
		const std::string healthText =
			(useChinese ? "生命 " : "HP ") +
			std::to_string(health) +
			"/" +
			std::to_string(maxHealth);
		const std::string waveText =
			"Wave " +
			std::to_string(snapshot.currentWave) +
			"/" +
			std::to_string(std::max(1, snapshot.maximumWaves));
		std::string ammoText =
			(useChinese ? "弹药 " : "Ammo ") +
			std::to_string(snapshot.player.ammoInMagazine) +
			"/" +
			std::to_string(snapshot.player.magazineCapacity);
		if (snapshot.player.reloading) {
			ammoText +=
				"  Reload " +
				std::to_string(static_cast<int>(
					snapshot.player.reloadProgress * 100.0f
				)) +
				"%";
		}
		const std::string scoreText =
			"Score " + std::to_string(snapshot.score);

		const SDL_Color healthColor{ 240, 240, 250, 255 };
		const SDL_Color waveColor{ 150, 220, 255, 255 };
		const SDL_Color ammoColor{ 255, 220, 120, 255 };
		const SDL_Color scoreColor{ 220, 220, 230, 255 };

		textRenderer_.drawLeft(
			renderer_,
			healthText,
			TextStyle::Body,
			16,
			10,
			healthColor
		);
		textRenderer_.drawCentered(
			renderer_,
			waveText,
			TextStyle::Body,
			outputWidth / 2,
			24,
			waveColor
		);
		textRenderer_.drawRight(
			renderer_,
			ammoText,
			TextStyle::Body,
			outputWidth - 140,
			10,
			ammoColor
		);
		textRenderer_.drawRight(
			renderer_,
			scoreText,
			TextStyle::Body,
			outputWidth - 16,
			34,
			scoreColor
		);

		SDL_SetRenderDrawBlendMode(
			renderer_,
			previousBlendMode
		);
	}

	void SdlGameRenderer::renderUpgradeOverlay(
		const GameSnapshot& snapshot) const {
		const CanvasSize size = canvasSize(renderer_);
		const bool chinese = chineseText();
		const auto cardRectangles =
			upgradeCardRects(renderer_);

		SDL_BlendMode previousBlendMode =
			SDL_BLENDMODE_NONE;
		SDL_GetRenderDrawBlendMode(
			renderer_,
			&previousBlendMode
		);
		SDL_SetRenderDrawBlendMode(
			renderer_,
			SDL_BLENDMODE_BLEND
		);

		const SDL_Rect screen{
			0,
			0,
			size.width,
			size.height
		};
		SDL_SetRenderDrawColor(
			renderer_,
			4,
			5,
			14,
			220
		);
		SDL_RenderFillRect(renderer_, &screen);

		textRenderer_.drawCentered(
			renderer_,
			chinese
				? "第 " + std::to_string(snapshot.currentWave) + " 波完成"
				: "WAVE " + std::to_string(snapshot.currentWave) + " COMPLETE",
			TextStyle::Title,
			size.width / 2,
			70,
			SDL_Color{ 255, 105, 165, 255 }
		);
		textRenderer_.drawCentered(
			renderer_,
			chinese ? "选择一项升级" : "CHOOSE ONE UPGRADE",
			TextStyle::Body,
			size.width / 2,
			116,
			SDL_Color{ 170, 225, 255, 255 }
		);

		for (int optionIndex = 0;
			optionIndex < snapshot.upgradeOptionCount &&
			optionIndex < static_cast<int>(cardRectangles.size());
			++optionIndex) {
			const UpgradeOptionSnapshot& option =
				snapshot.upgradeOptions[optionIndex];
			const SDL_Rect card =
				cardRectangles[optionIndex];
			const bool selected =
				optionIndex == selectedUpgradeOption_;

			SDL_SetRenderDrawColor(
				renderer_,
				selected ? 31 : 13,
				selected ? 45 : 23,
				selected ? 70 : 42,
				selected ? 248 : 238
			);
			SDL_RenderFillRect(renderer_, &card);
			SDL_SetRenderDrawColor(
				renderer_,
				selected ? 255 : 65,
				selected ? 85 : 170,
				selected ? 165 : 220,
				255
			);
			SDL_RenderDrawRect(renderer_, &card);
			if (selected) {
				const SDL_Rect inner{
					card.x + 3,
					card.y + 3,
					card.w - 6,
					card.h - 6
				};
				SDL_RenderDrawRect(renderer_, &inner);
			}

			std::string name;
			std::string description;
			std::vector<std::string> valueLines;
			std::ostringstream values;
			values << std::fixed << std::setprecision(2);

			switch (option.type) {
			case UpgradeType::HighVoltageRounds:
				name = chinese ? "高压弹药" : "HIGH-VOLTAGE ROUNDS";
				description = chinese
					? "强化弹头，提高单发伤害"
					: "INCREASE DAMAGE PER SHOT";
				valueLines.push_back(
					(chinese ? "伤害 " : "DAMAGE ") +
					std::to_string(snapshot.upgradeStats.projectileDamage) +
					" -> " +
					std::to_string(option.nextStats.projectileDamage)
				);
				break;
			case UpgradeType::FireRate:
				name = chinese ? "射速升级" : "FIRE RATE";
				description = chinese
					? "缩短射击间隔"
					: "REDUCE TIME BETWEEN SHOTS";
				valueLines.push_back(
					(chinese ? "射速加成 " : "RATE BONUS ") +
					std::to_string(option.currentLevel * 10) +
					"% -> " +
					std::to_string((option.currentLevel + 1) * 10) +
					"%"
				);
				break;
			case UpgradeType::AmmoSystem:
				name = chinese ? "弹药系统" : "AMMO SYSTEM";
				description = chinese
					? "扩大弹匣并加快换弹"
					: "MORE AMMO, FASTER RELOAD";
				valueLines.push_back(
					(chinese ? "弹匣 " : "MAGAZINE ") +
					std::to_string(snapshot.upgradeStats.magazineCapacity) +
					" -> " +
					std::to_string(option.nextStats.magazineCapacity)
				);
				values.str(std::string{});
				values.clear();
				values << std::fixed << std::setprecision(2)
					<< snapshot.upgradeStats.reloadDuration
					<< "s -> " << option.nextStats.reloadDuration << "s";
				valueLines.push_back(
					(chinese ? "换弹 " : "RELOAD ") + values.str()
				);
				break;
			case UpgradeType::ArmorCore:
				name = chinese ? "装甲核心" : "ARMOR CORE";
				description = chinese
					? "提升生命上限并恢复生命"
					: "GAIN MAX HEALTH AND HEAL";
				valueLines.push_back(
					(chinese ? "生命上限 " : "MAX HEALTH ") +
					std::to_string(snapshot.upgradeStats.maximumHealth) +
					" -> " +
					std::to_string(option.nextStats.maximumHealth)
				);
				valueLines.push_back(
					chinese ? "立即恢复 15" : "HEAL 15 NOW"
				);
				break;
			case UpgradeType::BallisticCalibration:
				name = chinese ? "弹道校准" : "BALLISTIC CALIBRATION";
				description = chinese
					? "降低散布并提高弹速"
					: "TIGHTER SPREAD, FASTER SHOTS";
				values.str(std::string{});
				values.clear();
				values << std::fixed << std::setprecision(1)
					<< snapshot.upgradeStats.projectileSpreadDegrees
					<< " -> " << option.nextStats.projectileSpreadDegrees;
				valueLines.push_back(
					(chinese ? "散布 " : "SPREAD ") + values.str()
				);
				valueLines.push_back(
					(chinese ? "弹速 " : "SHOT SPEED ") +
					std::to_string(static_cast<int>(
						snapshot.upgradeStats.projectileSpeed)) +
					" -> " +
					std::to_string(static_cast<int>(
						option.nextStats.projectileSpeed))
				);
				break;
			default:
				continue;
			}

			const int centerX = card.x + card.w / 2;
			const SDL_Color primaryColor = selected
				? SDL_Color{ 255, 225, 240, 255 }
				: SDL_Color{ 215, 230, 245, 255 };
			textRenderer_.drawCentered(
				renderer_,
				name,
				TextStyle::Body,
				centerX,
				card.y + 48,
				primaryColor
			);
			textRenderer_.drawCentered(
				renderer_,
				(chinese ? "等级 " : "LEVEL ") +
					std::to_string(option.currentLevel + 1) +
					" / " +
					std::to_string(option.maximumLevel),
				TextStyle::Body,
				centerX,
				card.y + 92,
				SDL_Color{ 110, 205, 255, 255 }
			);

			int lineY = card.y + 154;
			for (const std::string& line : valueLines) {
				textRenderer_.drawCentered(
					renderer_,
					line,
					TextStyle::Body,
					centerX,
					lineY,
					SDL_Color{ 255, 220, 125, 255 }
				);
				lineY += 42;
			}
			textRenderer_.drawCentered(
				renderer_,
				description,
				TextStyle::Body,
				centerX,
				card.y + card.h - 48,
				SDL_Color{ 190, 205, 220, 255 }
			);
		}

		textRenderer_.drawCentered(
			renderer_,
			chinese
				? "[A/D 或方向键] 选择       [Enter] 确认"
				: "[A/D OR ARROWS] SELECT       [ENTER] CONFIRM",
			TextStyle::Body,
			size.width / 2,
			size.height - 54,
			SDL_Color{ 235, 240, 250, 255 }
		);
		textRenderer_.drawCentered(
			renderer_,
			chinese ? "也可使用鼠标悬停并点击" : "MOUSE HOVER AND CLICK ALSO SUPPORTED",
			TextStyle::Body,
			size.width / 2,
			size.height - 22,
			SDL_Color{ 145, 180, 205, 255 }
		);

		SDL_SetRenderDrawBlendMode(
			renderer_,
			previousBlendMode
		);
	}

	void SdlGameRenderer::renderStateOverlay(
		const GameSnapshot& snapshot) const {
		const CanvasSize size = canvasSize(renderer_);
		const int outputWidth = size.width;
		const int outputHeight = size.height;
		if (outputWidth <= 0 || outputHeight <= 0) {
			return;
		}

		const bool paused =
			snapshot.state == GameState::Paused;
		const int panelWidth =
			paused
			? std::min(720, std::max(420, outputWidth - 60))
			: std::min(600, std::max(260, outputWidth - 40));
		const int panelHeight =
			paused
			? std::min(410, std::max(340, outputHeight - 60))
			: std::min(190, std::max(150, outputHeight - 40));

		const SDL_Rect panel{
			(outputWidth - panelWidth) / 2,
			(outputHeight - panelHeight) / 2,
			panelWidth,
			panelHeight
		};

		SDL_BlendMode previousBlendMode =
			SDL_BLENDMODE_NONE;
		SDL_GetRenderDrawBlendMode(
			renderer_,
			&previousBlendMode
		);
		SDL_SetRenderDrawBlendMode(
			renderer_,
			SDL_BLENDMODE_BLEND
		);

		SDL_SetRenderDrawColor(
			renderer_,
			12,
			12,
			20,
			235
		);
		SDL_RenderFillRect(renderer_, &panel);

		SDL_SetRenderDrawColor(
			renderer_,
			255,
			70,
			100,
			255
		);
		SDL_RenderDrawRect(renderer_, &panel);

		const bool useChinese = chineseText();
		if (paused) {
			const int centerX = panel.x + panel.w / 2;
			const int leftActionX =
				centerX - panel.w * 22 / 100;
			const int rightActionX =
				centerX + panel.w * 22 / 100;

			textRenderer_.drawCentered(
				renderer_,
				useChinese ? "游戏已暂停" : "GAME PAUSED",
				TextStyle::Title,
				centerX,
				panel.y + 54,
				SDL_Color{ 255, 105, 150, 255 }
			);
			textRenderer_.drawCentered(
				renderer_,
				(useChinese ? "当前波次：" : "CURRENT WAVE: ") +
					std::to_string(snapshot.currentWave) + " / " +
					std::to_string(std::max(1, snapshot.maximumWaves)),
				TextStyle::Body,
				centerX,
				panel.y + 122,
				SDL_Color{ 150, 220, 255, 255 }
			);
			textRenderer_.drawCentered(
				renderer_,
				(useChinese ? "当前分数：" : "CURRENT SCORE: ") +
					std::to_string(snapshot.score),
				TextStyle::Body,
				centerX,
				panel.y + 164,
				SDL_Color{ 235, 235, 245, 255 }
			);
			if (pauseMenuReturnEnabled_) {
				textRenderer_.drawCentered(
					renderer_,
					useChinese ? "是否返回主菜单？" :
						"RETURN TO THE MAIN MENU?",
					TextStyle::Body,
					centerX,
					panel.y + 226,
					SDL_Color{ 245, 245, 250, 255 }
				);
				textRenderer_.drawCentered(
					renderer_,
					useChinese ? "返回后本局进度不会保存" :
						"CURRENT RUN PROGRESS WILL NOT BE SAVED",
					TextStyle::Body,
					centerX,
					panel.y + 266,
					SDL_Color{ 255, 175, 90, 255 }
				);
				textRenderer_.drawCentered(
					renderer_,
					useChinese ? "[Enter] 返回主菜单" :
						"[ENTER] MAIN MENU",
					TextStyle::Body,
					leftActionX,
					panel.y + 350,
					SDL_Color{ 255, 220, 230, 255 }
				);
			}
			textRenderer_.drawCentered(
				renderer_,
				useChinese ? "[Esc] 继续游戏" :
					"[ESC] RESUME",
				TextStyle::Body,
				pauseMenuReturnEnabled_
					? rightActionX
					: centerX,
				panel.y + 350,
				SDL_Color{ 190, 225, 255, 255 }
			);

			SDL_SetRenderDrawBlendMode(
				renderer_,
				previousBlendMode
			);
			return;
		}

		std::string title;
		std::string prompt;
		if (snapshot.state == GameState::Intermission) {
			title = useChinese
				? "第 " + std::to_string(snapshot.currentWave) + " 波完成"
				: "WAVE " + std::to_string(snapshot.currentWave) + " COMPLETE";
			std::ostringstream timer;
			timer << std::fixed << std::setprecision(1)
				<< std::max(0.0f, snapshot.intermissionRemaining);
			prompt = useChinese
				? "下一波将在 " + timer.str() + " 秒后开始"
				: "NEXT WAVE IN " + timer.str() + "s";
		}
		else if (snapshot.state == GameState::Victory) {
			title = useChinese ? "挑战完成" : "VICTORY";
			prompt = useChinese
				? "[R] 重新开始    [Esc] 返回主菜单"
				: "[R] RESTART    [ESC] MAIN MENU";
		}
		else {
			title = useChinese ? "玩家已死亡" : "GAME OVER";
			prompt = useChinese
				? "[R] 重新开始    [Esc] 返回主菜单"
				: "[R] RESTART    [ESC] MAIN MENU";
		}

		const SDL_Color titleColor{
			255,
			110,
			130,
			255
		};
		const SDL_Color promptColor{
			240,
			240,
			250,
			255
		};

		textRenderer_.drawCentered(
			renderer_,
			title,
			TextStyle::Title,
			panel.x + panel.w / 2,
			panel.y + panel.h / 2 - 34,
			titleColor
		);
		textRenderer_.drawCentered(
			renderer_,
			prompt,
			TextStyle::Body,
			panel.x + panel.w / 2,
			panel.y + panel.h / 2 + 34,
			promptColor
		);

		SDL_SetRenderDrawBlendMode(
			renderer_,
			previousBlendMode
		);
	}

	void SdlGameRenderer::render(
		const GameSnapshot& snapshot,
		const std::vector<PresentationEvent>& presentationEvents,
		float frameDt) {
		if (snapshot.state == GameState::Playing &&
			(lastRenderedState_ == GameState::Gameover ||
				lastRenderedState_ == GameState::Victory)) {
			presentationEffects_.reset();
		}

		presentationEffects_.update(
			frameDt,
			presentationEvents
		);

		const auto drawRectangle =
			[this](
				const Rect& bounds,
				Uint8 red,
				Uint8 green,
				Uint8 blue) {

					const SDL_FRect rectangle{
						bounds.x,
						bounds.y,
						bounds.w,
						bounds.h
					};

					SDL_SetRenderDrawColor(
						renderer_,
						red,
						green,
						blue,
						255
					);
					SDL_RenderFillRectF(
						renderer_,
						&rectangle
					);
			};

		const auto drawTextureOrRectangle =
			[this, &drawRectangle](
				TextureId textureId,
				const Rect& bounds,
				Uint8 red,
				Uint8 green,
				Uint8 blue,
				Uint8 modulationRed,
				Uint8 modulationGreen,
				Uint8 modulationBlue) {

					SDL_Texture* texture =
						assetManager_.texture(textureId);

					if (texture != nullptr) {
						SDL_SetTextureColorMod(
							texture,
							modulationRed,
							modulationGreen,
							modulationBlue
						);
						const SDL_FRect destination{
							bounds.x,
							bounds.y,
							bounds.w,
							bounds.h
						};

						const bool rendered = SDL_RenderCopyF(
							renderer_,
							texture,
							nullptr,
							&destination
						) == 0;
						SDL_SetTextureColorMod(
							texture,
							255,
							255,
							255
						);

						if (rendered) {
							return;
						}
					}

					drawRectangle(
						bounds,
						red,
						green,
						blue
					);
			};

		const MapVisualDefinition& mapVisual =
			mapVisualFor(snapshot.mapId);

		SDL_SetRenderDrawColor(
			renderer_,
			mapVisual.backgroundFallback.red,
			mapVisual.backgroundFallback.green,
			mapVisual.backgroundFallback.blue,
			255
		);

		SDL_RenderClear(renderer_);

		if (mapVisual.backgroundTexture.has_value()) {
			SDL_Texture* backgroundTexture =
				assetManager_.texture(
					*mapVisual.backgroundTexture
				);
			if (backgroundTexture != nullptr) {
				const CanvasSize size = canvasSize(renderer_);
				const SDL_FRect destination{
					0.0f,
					0.0f,
					static_cast<float>(size.width),
					static_cast<float>(size.height)
				};
				SDL_SetTextureColorMod(
					backgroundTexture,
					180,
					180,
					180
				);
				SDL_RenderCopyF(
					renderer_,
					backgroundTexture,
					nullptr,
					&destination
				);
				SDL_SetTextureColorMod(
					backgroundTexture,
					255,
					255,
					255
				);
			}
		}

		for (const ObstacleSnapshot& obstacle :
			snapshot.obstacles) {
			drawRectangle(
				obstacle.bounds,
				12,
				14,
				24
			);

			bool rendered = false;
			if (mapVisual.obstacleTexture.has_value()) {
				SDL_Texture* obstacleTexture =
					assetManager_.texture(
						*mapVisual.obstacleTexture
					);

				if (obstacleTexture != nullptr) {
					const bool vertical =
						obstacle.bounds.h > obstacle.bounds.w;
					const SDL_FRect destination = vertical
						? SDL_FRect{
							obstacle.bounds.x +
								(obstacle.bounds.w - obstacle.bounds.h) /
								2.0f,
							obstacle.bounds.y +
								(obstacle.bounds.h - obstacle.bounds.w) /
								2.0f,
							obstacle.bounds.h,
							obstacle.bounds.w
						}
						: SDL_FRect{
							obstacle.bounds.x,
							obstacle.bounds.y,
							obstacle.bounds.w,
							obstacle.bounds.h
						};

					rendered = SDL_RenderCopyExF(
						renderer_,
						obstacleTexture,
						nullptr,
						&destination,
						vertical ? 90.0 : 0.0,
						nullptr,
						SDL_FLIP_NONE
					) == 0;
				}
			}

			if (!rendered) {
				drawRectangle(
					obstacle.bounds,
					mapVisual.obstacleFallback.red,
					mapVisual.obstacleFallback.green,
					mapVisual.obstacleFallback.blue
				);
			}

			SDL_SetRenderDrawColor(
				renderer_,
				mapVisual.obstacleFallback.red,
				mapVisual.obstacleFallback.green,
				mapVisual.obstacleFallback.blue,
				255
			);

			for (int inset = 0; inset < 3; ++inset) {
				const float insetValue =
					static_cast<float>(inset);
				const SDL_FRect outline{
					obstacle.bounds.x + insetValue,
					obstacle.bounds.y + insetValue,
					obstacle.bounds.w - insetValue * 2.0f,
					obstacle.bounds.h - insetValue * 2.0f
				};

				if (outline.w > 0.0f && outline.h > 0.0f) {
					SDL_RenderDrawRectF(
						renderer_,
						&outline
					);
				}
			}
		}

		for (const EnemySnapshot& enemy :
			snapshot.enemies) {

			const bool isShooter =
				enemy.kind == EnemyKind::Shooter;

			const Uint8 shooterGreen =
				enemy.warningActive
				? static_cast<Uint8>(
					150.0f +
					105.0f * enemy.warningProgress
					)
				: 150;

			drawTextureOrRectangle(
				isShooter
				? TextureId::EnemyShooter
				: TextureId::EnemyChaser,
				enemy.bounds,
				isShooter ? 255 : 220,
				isShooter ? shooterGreen : 60,
				isShooter ? 40 : 70,
				255,
				255,
				enemy.warningActive ? 160 : 255
			);
		}

		drawTextureOrRectangle(
			TextureId::Player,
			snapshot.player.bounds,
			snapshot.player.invulnerable ? 255 : 0,
			snapshot.player.invulnerable ? 255 : 220,
			255,
			255,
			255,
			snapshot.player.invulnerable ? 150 : 255
		);

		for (const ProjectileSnapshot& projectile :
			snapshot.projectiles) {

			const bool isEnemyProjectile =
				projectile.faction ==
				ProjectileFaction::Enemy;

			drawTextureOrRectangle(
				isEnemyProjectile
				? TextureId::EnemyProjectile
				: TextureId::PlayerProjectile,
				projectile.bounds,
				isEnemyProjectile ? 255 : 255,
				isEnemyProjectile ? 70 : 220,
				isEnemyProjectile ? 180 : 80,
				255,
				255,
				255
			);
		}

		const Vec2 playerCenter{
			snapshot.player.bounds.x +
			snapshot.player.bounds.w / 2.0f,
			snapshot.player.bounds.y +
			snapshot.player.bounds.h / 2.0f
		};

		Vec2 aimDirection{
			snapshot.aimPosition.x -
			playerCenter.x,
			snapshot.aimPosition.y -
			playerCenter.y
		};

		const float aimLength =
			std::sqrt(
				aimDirection.x * aimDirection.x +
				aimDirection.y * aimDirection.y
			);

		if (aimLength > 0.0f) {
			aimDirection.x /= aimLength;
			aimDirection.y /= aimLength;

			constexpr float lineLength = 60.0f;

			SDL_SetRenderDrawColor(
				renderer_,
				255,
				80,
				160,
				255
			);

			SDL_RenderDrawLineF(
				renderer_,
				playerCenter.x,
				playerCenter.y,
				playerCenter.x +
				aimDirection.x * lineLength,
				playerCenter.y +
				aimDirection.y * lineLength
			);
		}

		presentationEffects_.render(
			renderer_,
			textRenderer_
		);

		renderHud(snapshot);

		if (snapshot.state == GameState::UpgradeSelection) {
			renderUpgradeOverlay(snapshot);
		}
		else if (snapshot.state != GameState::Playing) {
			renderStateOverlay(snapshot);
		}

		SDL_RenderPresent(renderer_);
		lastRenderedState_ = snapshot.state;
	}

}//namespace neon::sdl
