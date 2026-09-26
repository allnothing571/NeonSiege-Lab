#define SDL_MAIN_HANDLED

#include "GameApplication.h"

#include <SDL.h>
#include <SDL_image.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <string>

#include "SaveData.h"
#include "core/GameSnapshot.h"
#include "core/GameplayConfig.h"
#include "core/InputCommand.h"
#include "core/MapCatalog.h"
#include "core/Simulation.h"
#include "sdl/AssetManager.h"
#include "sdl/SdlAssetCatalog.h"
#include "sdl/SdlAppSettings.h"
#include "sdl/SdlGameRenderer.h"
#include "sdl/SdlInput.h"
#include "sdl/SdlPaths.h"

namespace {

	enum class AppScreen {
		MainMenu,
		HowToPlay,
		Settings,
		Gameplay
	};

	bool isTerminalState(
		neon::GameState state) {
		return state == neon::GameState::Gameover ||
			state == neon::GameState::Victory;
	}

	int wrappedSelection(
		int selection,
		int delta,
		int itemCount) {
		return (selection + delta + itemCount) % itemCount;
	}

	void adjustSettingsValue(
		neon::sdl::AppSettings& settings,
		int selectedItem,
		int delta) {
		if (selectedItem == 0) {
			settings.language =
				settings.language == neon::sdl::UiLanguage::Chinese
				? neon::sdl::UiLanguage::English
				: neon::sdl::UiLanguage::Chinese;
		}
		else if (selectedItem == 1) {
			settings.windowSizeIndex = wrappedSelection(
				settings.windowSizeIndex,
				delta,
				static_cast<int>(
					neon::sdl::windowSizePresets().size()
				)
			);
		}
		else if (selectedItem == 2) {
			settings.windowMode =
				settings.windowMode ==
					neon::sdl::WindowMode::Windowed
				? neon::sdl::WindowMode::BorderlessFullscreen
				: neon::sdl::WindowMode::Windowed;
		}
	}

}//namespace

int runNeonSiege(
	bool smokeTest) {
	const neon::GameplayConfig config{};

	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
		return 1;
	}

	const std::string settingsPath = smokeTest
		? std::string{}
		: neon::sdl::preferenceFilePath("settings.txt");
	const bool settingsPersistenceEnabled =
		!settingsPath.empty();
	neon::sdl::AppSettings settings = smokeTest
		? neon::sdl::AppSettings{}
		: neon::sdl::loadAppSettings(settingsPath);
	neon::sdl::AppSettings pendingSettings = settings;
	const auto initialSize =
		neon::sdl::windowSizePresets()[settings.windowSizeIndex];

	Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
	windowFlags |= smokeTest
		? SDL_WINDOW_HIDDEN
		: SDL_WINDOW_SHOWN;
	SDL_Window* window = SDL_CreateWindow(
		"Neon Siege",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		initialSize.width,
		initialSize.height,
		windowFlags
	);
	if (window == nullptr) {
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
		SDL_Quit();
		return 1;
	}

	if (!smokeTest &&
		!neon::sdl::applyAppSettings(window, settings)) {
		std::cerr << "无法应用窗口设置: " << SDL_GetError() << '\n';
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED |
			SDL_RENDERER_PRESENTVSYNC
	);
	if (renderer == nullptr) {
		renderer = SDL_CreateRenderer(
			window,
			-1,
			SDL_RENDERER_SOFTWARE
		);
	}
	if (renderer == nullptr) {
		std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	if (SDL_RenderSetLogicalSize(
		renderer,
		static_cast<int>(config.worldBounds.w),
		static_cast<int>(config.worldBounds.h)) != 0) {
		std::cerr << "无法设置逻辑画布: " << SDL_GetError() << '\n';
	}

	const int requestedImageFlags = IMG_INIT_PNG;
	const int initializedImageFlags =
		IMG_Init(requestedImageFlags);
	if ((initializedImageFlags & requestedImageFlags) !=
		requestedImageFlags) {
		std::cerr << "SDL_image PNG 初始化警告: "
			<< IMG_GetError() << '\n';
	}

	neon::sdl::AssetManager assetManager{};
	const neon::sdl::AssetLoadReport assetLoadReport =
		neon::sdl::loadGameAssets(
			renderer,
			assetManager
		);
	for (const neon::sdl::AssetLoadIssue& issue :
		assetLoadReport.issues) {
		std::cerr << "素材加载失败，使用图形回退: "
			<< issue.relativePath
			<< " (" << issue.message << ")\n";
	}
	neon::sdl::SdlGameRenderer gameRenderer(
		renderer,
		assetManager
	);
	gameRenderer.setLanguage(settings.language);
	gameRenderer.setPauseMenuReturnEnabled(true);
	neon::sdl::SdlInput sdlInput(renderer);
	neon::Simulation simulation(config);

	const double frequency =
		static_cast<double>(SDL_GetPerformanceFrequency());
	Uint64 previousCounter = SDL_GetPerformanceCounter();
	constexpr double fixedDt = 1.0 / 60.0;
	double accumulator = 0.0;
	bool pendingReloadPressed = false;
	bool running = true;

	AppScreen screen = smokeTest
		? AppScreen::Gameplay
		: AppScreen::MainMenu;
	int mainMenuSelection = 0;
	int settingsSelection = 0;
	int upgradeSelection = 1;
	bool upgradeScreenActive = false;
	constexpr double upgradeInputLockDuration = 0.30;
	double upgradeInputLockRemaining = 0.0;
	bool upgradeInputArmed = false;
	int upgradePointerPressedOption = -1;
	bool howToPlayBackHovered = false;
	std::mt19937 mapSelectionEngine{
		std::random_device{}()
	};
	std::optional<neon::MapId> previousMapId;

	SaveData saveData;
	const std::string highScorePath = smokeTest
		? std::string{}
		: neon::sdl::preferenceFilePath("high_score.txt");
	const bool persistenceEnabled = !highScorePath.empty();
	int highScore = persistenceEnabled
		? saveData.loadHighScore(highScorePath)
		: 0;

	auto returnToMainMenu = [&]() {
		screen = AppScreen::MainMenu;
		mainMenuSelection = 0;
		pendingReloadPressed = false;
		upgradeSelection = 1;
		upgradeScreenActive = false;
		upgradeInputLockRemaining = 0.0;
		upgradeInputArmed = false;
		upgradePointerPressedOption = -1;
		accumulator = 0.0;
		simulation.consumePresentationEvents();
		gameRenderer.resetPresentationEffects();
		SDL_SetWindowTitle(window, "Neon Siege");
	};

	auto startGame = [&]() {
		const neon::MapId selectedMap =
			neon::chooseRandomPresetMap(
				mapSelectionEngine,
				previousMapId
			);
		if (simulation.reset(selectedMap)) {
			previousMapId = selectedMap;
		}
		else {
			simulation.reset();
		}
		simulation.consumePresentationEvents();
		gameRenderer.resetPresentationEffects();
		pendingReloadPressed = false;
		upgradeSelection = 1;
		upgradeScreenActive = false;
		upgradeInputLockRemaining = 0.0;
		upgradeInputArmed = false;
		upgradePointerPressedOption = -1;
		accumulator = 0.0;
		previousCounter = SDL_GetPerformanceCounter();
		screen = AppScreen::Gameplay;
	};

	while (running) {
		const Uint64 currentCounter = SDL_GetPerformanceCounter();
		constexpr double maxFrameTime = 0.25;
		const double rawFrameTime =
			(currentCounter - previousCounter) / frequency;
		const double frameTime =
			std::min(rawFrameTime, maxFrameTime);
		previousCounter = currentCounter;

		SDL_Event event{};
		while (SDL_PollEvent(&event) != 0) {
			sdlInput.handleEvent(event);
		}
		if (sdlInput.quitRequested()) {
			break;
		}

		neon::InputCommand command =
			sdlInput.consumeCommand();
		const neon::sdl::UiCommand uiCommand =
			sdlInput.consumeUiCommand();

		if (screen == AppScreen::MainMenu) {
			if (uiCommand.pointerMoved) {
				const int hoveredItem = gameRenderer.mainMenuItemAt(
					uiCommand.pointerX,
					uiCommand.pointerY
				);
				if (hoveredItem >= 0) {
					mainMenuSelection = hoveredItem;
				}
			}
			if (uiCommand.upPressed) {
				mainMenuSelection =
					wrappedSelection(mainMenuSelection, -1, 4);
			}
			if (uiCommand.downPressed) {
				mainMenuSelection =
					wrappedSelection(mainMenuSelection, 1, 4);
			}

			bool activate = uiCommand.confirmPressed;
			if (uiCommand.pointerPressed) {
				const int clickedItem = gameRenderer.mainMenuItemAt(
					uiCommand.pointerX,
					uiCommand.pointerY
				);
				if (clickedItem >= 0) {
					mainMenuSelection = clickedItem;
					activate = true;
				}
			}

			if (activate) {
				if (mainMenuSelection == 0) {
					startGame();
				}
				else if (mainMenuSelection == 1) {
					howToPlayBackHovered = false;
					screen = AppScreen::HowToPlay;
				}
				else if (mainMenuSelection == 2) {
					pendingSettings = settings;
					settingsSelection = 0;
					screen = AppScreen::Settings;
				}
				else {
					running = false;
				}
			}

			if (screen == AppScreen::MainMenu && running) {
				gameRenderer.renderMainMenu(mainMenuSelection);
			}
			continue;
		}

		if (screen == AppScreen::HowToPlay) {
			if (uiCommand.pointerMoved) {
				howToPlayBackHovered =
					gameRenderer.howToPlayBackAt(
						uiCommand.pointerX,
						uiCommand.pointerY
					);
			}

			if (uiCommand.backPressed ||
				uiCommand.confirmPressed ||
				(uiCommand.pointerPressed &&
					gameRenderer.howToPlayBackAt(
						uiCommand.pointerX,
						uiCommand.pointerY))) {
				screen = AppScreen::MainMenu;
				gameRenderer.renderMainMenu(mainMenuSelection);
			}
			else {
				gameRenderer.renderHowToPlay(
					howToPlayBackHovered
				);
			}
			continue;
		}

		if (screen == AppScreen::Settings) {
			if (uiCommand.backPressed) {
				pendingSettings = settings;
				gameRenderer.setLanguage(settings.language);
				screen = AppScreen::MainMenu;
				gameRenderer.renderMainMenu(mainMenuSelection);
				continue;
			}

			if (uiCommand.pointerMoved) {
				const int hoveredItem = gameRenderer.settingsItemAt(
					uiCommand.pointerX,
					uiCommand.pointerY
				);
				if (hoveredItem >= 0) {
					settingsSelection = hoveredItem;
				}
			}

			if (uiCommand.upPressed) {
				settingsSelection =
					wrappedSelection(settingsSelection, -1, 5);
			}
			if (uiCommand.downPressed) {
				settingsSelection =
					wrappedSelection(settingsSelection, 1, 5);
			}
			if (uiCommand.leftPressed) {
				adjustSettingsValue(
					pendingSettings,
					settingsSelection,
					-1
				);
			}
			if (uiCommand.rightPressed) {
				adjustSettingsValue(
					pendingSettings,
					settingsSelection,
					1
				);
			}

			bool activate = uiCommand.confirmPressed;
			if (uiCommand.pointerPressed) {
				const int clickedItem = gameRenderer.settingsItemAt(
					uiCommand.pointerX,
					uiCommand.pointerY
				);
				if (clickedItem >= 0) {
					settingsSelection = clickedItem;
					activate = true;
				}
			}

			if (activate && settingsSelection < 3) {
				adjustSettingsValue(
					pendingSettings,
					settingsSelection,
					1
				);
			}
			else if (activate && settingsSelection == 3) {
				settings = pendingSettings;
				gameRenderer.setLanguage(settings.language);
				if (!neon::sdl::applyAppSettings(window, settings)) {
					std::cerr << "无法应用窗口设置: "
						<< SDL_GetError() << '\n';
				}
				if (settingsPersistenceEnabled &&
					!neon::sdl::saveAppSettings(
						settingsPath,
						settings)) {
					std::cerr << "无法保存设置\n";
				}
				screen = AppScreen::MainMenu;
				gameRenderer.renderMainMenu(mainMenuSelection);
				continue;
			}
			else if (activate && settingsSelection == 4) {
				pendingSettings = settings;
				gameRenderer.setLanguage(settings.language);
				screen = AppScreen::MainMenu;
				gameRenderer.renderMainMenu(mainMenuSelection);
				continue;
			}

			gameRenderer.setLanguage(pendingSettings.language);
			gameRenderer.renderSettings(
				pendingSettings,
				settingsSelection
			);
			continue;
		}

		const neon::GameState stateBeforeInput =
			simulation.state();
		if ((stateBeforeInput == neon::GameState::Paused &&
				uiCommand.confirmPressed) ||
			(isTerminalState(stateBeforeInput) &&
				uiCommand.backPressed)) {
			returnToMainMenu();
			gameRenderer.renderMainMenu(mainMenuSelection);
			continue;
		}

		if (stateBeforeInput ==
			neon::GameState::UpgradeSelection) {
			const neon::GameSnapshot upgradeSnapshot =
				simulation.snapshot();
			const int optionCount = std::max(
				1,
				upgradeSnapshot.upgradeOptionCount
			);

			if (!upgradeScreenActive) {
				upgradeSelection =
					std::min(1, optionCount - 1);
				upgradeScreenActive = true;
				upgradeInputLockRemaining =
					upgradeInputLockDuration;
				upgradeInputArmed = false;
				upgradePointerPressedOption = -1;
			}

			if (upgradeInputLockRemaining > 0.0) {
				upgradeInputLockRemaining = std::max(
					0.0,
					upgradeInputLockRemaining - frameTime
				);
				upgradePointerPressedOption = -1;
			}
			else if (!command.fireHeld) {
				upgradeInputArmed = true;
			}

			if (uiCommand.pointerMoved) {
				const int hoveredOption =
					gameRenderer.upgradeOptionAt(
						uiCommand.pointerX,
						uiCommand.pointerY
					);
				if (hoveredOption >= 0 &&
					hoveredOption < optionCount) {
					upgradeSelection = hoveredOption;
				}
			}

			if (uiCommand.leftPressed) {
				upgradeSelection = wrappedSelection(
					upgradeSelection,
					-1,
					optionCount
				);
			}
			if (uiCommand.rightPressed) {
				upgradeSelection = wrappedSelection(
					upgradeSelection,
					1,
					optionCount
				);
			}

			bool confirmUpgrade =
				upgradeInputArmed &&
				uiCommand.confirmPressed;
			if (upgradeInputArmed &&
				uiCommand.pointerPressed) {
				const int pressedOption =
					gameRenderer.upgradeOptionAt(
						uiCommand.pointerPressedX,
						uiCommand.pointerPressedY
					);
				if (pressedOption >= 0 &&
					pressedOption < optionCount) {
					upgradeSelection = pressedOption;
					upgradePointerPressedOption =
						pressedOption;
				}
				else {
					upgradePointerPressedOption = -1;
				}
			}

			if (upgradeInputArmed &&
				uiCommand.pointerReleased) {
				const int releasedOption =
					gameRenderer.upgradeOptionAt(
						uiCommand.pointerReleasedX,
						uiCommand.pointerReleasedY
					);
				confirmUpgrade =
					upgradePointerPressedOption >= 0 &&
					releasedOption ==
						upgradePointerPressedOption;
				if (confirmUpgrade) {
					upgradeSelection = releasedOption;
				}
				upgradePointerPressedOption = -1;
			}

			if (confirmUpgrade) {
				command.upgradeSelection =
					upgradeSelection;
				upgradeInputArmed = false;
			}
			pendingReloadPressed = false;
		}
		else if (stateBeforeInput !=
			neon::GameState::Paused) {
			upgradeScreenActive = false;
			upgradeInputLockRemaining = 0.0;
			upgradeInputArmed = false;
			upgradePointerPressedOption = -1;
		}

		gameRenderer.setUpgradeSelection(
			upgradeSelection
		);

		pendingReloadPressed =
			pendingReloadPressed ||
			(stateBeforeInput == neon::GameState::Playing &&
				command.reloadPressed);
		command.reloadPressed = pendingReloadPressed;

		const bool stateCommand =
			command.pausePressed ||
			command.upgradeSelection >= 0 ||
			(command.restartPressed &&
				isTerminalState(simulation.state()));
		if (stateCommand) {
			simulation.step(
				command,
				static_cast<float>(fixedDt)
			);
			pendingReloadPressed = false;
			command.pausePressed = false;
			command.restartPressed = false;
			command.reloadPressed = false;
			command.upgradeSelection = -1;
			accumulator = 0.0;
		}
		else if (
			simulation.state() == neon::GameState::Playing ||
			simulation.state() == neon::GameState::Intermission) {
			accumulator += frameTime;
		}

		while ((simulation.state() == neon::GameState::Playing ||
			simulation.state() == neon::GameState::Intermission) &&
			accumulator >= fixedDt) {
			simulation.step(
				command,
				static_cast<float>(fixedDt)
			);
			pendingReloadPressed = false;
			command.pausePressed = false;
			command.restartPressed = false;
			command.reloadPressed = false;
			accumulator -= fixedDt;
		}

		const neon::GameSnapshot snapshot =
			simulation.snapshot();
		const auto presentationEvents =
			simulation.consumePresentationEvents();

		if (isTerminalState(snapshot.state) &&
			snapshot.score > highScore) {
			highScore = snapshot.score;
			if (persistenceEnabled &&
				!saveData.saveHighScore(
					highScorePath,
					highScore)) {
				std::cerr << "无法保存最高分\n";
			}
		}

		const int displayedHighScore =
			std::max(highScore, snapshot.score);
		const std::string title =
			"Neon Siege | Wave " +
			std::to_string(snapshot.currentWave) +
			" | HP: " +
			std::to_string(snapshot.player.health) +
			" | Ammo: " +
			std::to_string(snapshot.player.ammoInMagazine) +
			"/" +
			std::to_string(snapshot.player.magazineCapacity) +
			" | Score: " +
			std::to_string(snapshot.score) +
			" | High: " +
			std::to_string(displayedHighScore);
		SDL_SetWindowTitle(window, title.c_str());

		gameRenderer.render(
			snapshot,
			presentationEvents,
			static_cast<float>(frameTime)
		);

		if (smokeTest) {
			running = false;
		}
	}

	if (persistenceEnabled &&
		!saveData.saveHighScore(highScorePath, highScore)) {
		std::cerr << "无法保存最高分\n";
	}

	assetManager.clear();
	IMG_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
