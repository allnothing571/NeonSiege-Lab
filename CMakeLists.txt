cmake_minimum_required(VERSION 3.21)

project(NeonSiege VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
option(
    BUILD_TESTING
    "Build and register tests"
    ON
)

if(BUILD_TESTING)
    enable_testing()
endif()

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL2TTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_BMP OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_GIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JPG OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_LBM OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_PCX OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_PNG ON CACHE BOOL "" FORCE)
set(SDL2IMAGE_PNM OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_QOI OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SVG OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TGA OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_XCF OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_XPM OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_XV OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    SDL2_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-2.8.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2_image)

FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.24.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES external/freetype
    GIT_SUBMODULES_RECURSE TRUE
)
FetchContent_MakeAvailable(SDL2_ttf)

add_library(neon_core STATIC)

target_include_directories(
    neon_core
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(
    neon_core
    PRIVATE
    src/core/Types.h
    src/core/InputCommand.h
    src/core/GameState.h
    src/core/GameplayConfig.h
    src/core/UpgradeType.h
    src/core/UpgradeSystem.h
    src/core/UpgradeSystem.cpp
    src/core/WaveDifficulty.h
    src/core/EnemyKind.h
    src/core/GameSnapshot.h
    src/core/EntityId.h
    src/core/GameEvent.h
    src/core/PresentationEvent.h
    src/core/ReplayFrame.h
    src/core/ReplayTape.h
    src/core/ReplayTape.cpp
    src/core/DetectionEvent.h
    src/core/SpeedDetector.h
    src/core/SpeedDetector.cpp
    src/core/AutoFireDetector.h
    src/core/AutoFireDetector.cpp
    src/core/TeleportDetector.h
    src/core/TeleportDetector.cpp
    src/core/HealthDetector.h
    src/core/HealthDetector.cpp
    src/core/DamageDetector.h
    src/core/DamageDetector.cpp
    src/core/ScoreDetector.h
    src/core/ScoreDetector.cpp
    src/core/AimAssistDetector.h
    src/core/AimAssistDetector.cpp
    src/core/EspDetector.h
    src/core/EspDetector.cpp
    src/core/AutoDodgeDetector.h
    src/core/AutoDodgeDetector.cpp
    src/core/Collision.h
    src/core/Obstacle.h
    src/core/MapDefinition.h
    src/core/MapCatalog.h
    src/core/MapCatalog.cpp
    src/core/LineOfSight.h
    src/core/AimSpread.h
    src/core/SpawnPlacement.h
    src/core/SpawnPlacement.cpp
    src/core/Weapon.h
    src/core/Player.h
    src/core/Projectile.h
    src/core/Enemy.h
    src/core/WaveManager.h
    src/core/Simulation.h
    src/core/Simulation.cpp
)

if(MSVC)
    target_compile_options(
        neon_core
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_library(
    neon_sdl
    STATIC
    src/sdl/SdlGameRenderer.h
    src/sdl/SdlGameRenderer.cpp
    src/sdl/SdlPaths.h
    src/sdl/SdlPaths.cpp
    src/sdl/SdlAppSettings.h
    src/sdl/SdlAppSettings.cpp
    src/sdl/SdlInput.h
    src/sdl/SdlInput.cpp
    src/sdl/AssetManager.h
    src/sdl/AssetManager.cpp
    src/sdl/SdlAssetCatalog.h
    src/sdl/SdlAssetCatalog.cpp
    src/sdl/SdlTextRenderer.h
    src/sdl/SdlTextRenderer.cpp
    src/sdl/SdlPresentationEffects.h
    src/sdl/SdlPresentationEffects.cpp
)

target_include_directories(
    neon_sdl
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    neon_sdl
    PUBLIC neon_core
    PRIVATE
    SDL2::SDL2-static
    SDL2_image::SDL2_image-static
    SDL2_ttf::SDL2_ttf-static
)

if(MSVC)
    target_compile_options(
        neon_sdl
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
NeonSiege
src/main.cpp
src/GameApplication.h
src/GameApplication.cpp
src/SaveData.h
src/SaveData.cpp)

add_custom_command(
    TARGET NeonSiege
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:NeonSiege>/assets"
)

add_executable(
    Day7CollisionRewrite
    exercises/day7_collision_rewrite.cpp)

if(MSVC)
    target_compile_options(
        Day7CollisionRewrite
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7EnemyObstacleDamage
    exercises/day7_enemy_obstacle_damage.cpp
)

add_executable(
    CommercialMapCatalog
    exercises/commercial_map_catalog.cpp
)

add_executable(
    CommercialAssetCatalog
    exercises/commercial_asset_catalog.cpp
)

add_executable(
    CommercialUpgradeSystem
    exercises/commercial_upgrade_system.cpp
)

target_link_libraries(
    CommercialUpgradeSystem
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        CommercialUpgradeSystem
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    CommercialWaveDifficulty
    exercises/commercial_wave_difficulty.cpp
)

target_link_libraries(
    CommercialWaveDifficulty
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        CommercialWaveDifficulty
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

target_link_libraries(
    CommercialAssetCatalog
    PRIVATE
    neon_sdl
)

if(MSVC)
    target_compile_options(
        CommercialAssetCatalog
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

target_link_libraries(
    CommercialMapCatalog
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        CommercialMapCatalog
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

target_link_libraries(
    Day7EnemyObstacleDamage
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7EnemyObstacleDamage
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7AutoReload
    exercises/day7_auto_reload.cpp
)

target_link_libraries(
    Day7AutoReload
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7AutoReload
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7SpawnSafety
    exercises/day7_spawn_safety.cpp
)

target_link_libraries(
    Day7SpawnSafety
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7SpawnSafety
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7EnemyBoundary
    exercises/day7_enemy_boundary.cpp
)

target_link_libraries(
    Day7EnemyBoundary
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7EnemyBoundary
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7EnemyNavigation
    exercises/day7_enemy_navigation.cpp
)

target_link_libraries(
    Day7EnemyNavigation
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7EnemyNavigation
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7GameOverOverlay
    exercises/day7_game_over_overlay.cpp
)

target_link_libraries(
    Day7GameOverOverlay
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7GameOverOverlay
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7FrontendStates
    exercises/day7_frontend_states.cpp
)

target_link_libraries(
    Day7FrontendStates
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7FrontendStates
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7PresentationEvents
    exercises/day7_presentation_events.cpp
)

target_link_libraries(
    Day7PresentationEvents
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day7PresentationEvents
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day7FrontendInput
    exercises/day7_frontend_input.cpp
)

target_link_libraries(
    Day7FrontendInput
    PRIVATE
    neon_sdl
    SDL2::SDL2-static
)

if(MSVC)
    target_compile_options(
        Day7FrontendInput
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day1EntityExercise
    exercises/day1_entity_manager.cpp)

    add_executable(
    Day1CoreTypes
    exercises/day1_core_types.cpp)

target_link_libraries(Day1CoreTypes PRIVATE neon_core)

add_executable(
    Day1CoreCollision
    exercises/day1_core_collision.cpp)

target_link_libraries(Day1CoreCollision PRIVATE neon_core)

add_executable(
    Day1CorePlayer
    exercises/day1_core_player.cpp
)

target_link_libraries(
    Day1CorePlayer
    PRIVATE
    neon_core
)

add_executable(
    Day1CoreProjectile
    exercises/day1_core_projectile.cpp
)

target_link_libraries(
    Day1CoreProjectile
    PRIVATE
    neon_core
)

add_executable(
    Day1CoreEnemy
    exercises/day1_core_enemy.cpp
)

target_link_libraries(
    Day1CoreEnemy
    PRIVATE
    neon_core
)

add_executable(
    Day1CoreWaveManager
    exercises/day1_core_wave_manager.cpp
)

target_link_libraries(
    Day1CoreWaveManager
    PRIVATE
    neon_core
)

add_executable(
    Day2InputCommand
    exercises/day2_input_command.cpp
)

target_link_libraries(
    Day2InputCommand
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day2InputCommand
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day2GameplayConfig
    exercises/day2_gameplay_config.cpp
)

target_link_libraries(
    Day2GameplayConfig
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day2GameplayConfig
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day2Simulation
    exercises/day2_simulation.cpp
)

target_link_libraries(
    Day2Simulation
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day2Simulation
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day3Obstacle
    exercises/day3_obstacle.cpp
)

target_link_libraries(
    Day3Obstacle
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day3Obstacle
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day3LineOfSight
    exercises/day3_line_of_sight.cpp
)

target_link_libraries(
    Day3LineOfSight
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day3LineOfSight
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day3SpawnPlacement
    exercises/day3_spawn_placement.cpp
)

target_link_libraries(
    Day3SpawnPlacement
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day3SpawnPlacement
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day3Weapon
    exercises/day3_weapon.cpp
)

target_link_libraries(
    Day3Weapon
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day3Weapon
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day3AimSpread
    exercises/day3_aim_spread.cpp
)

target_link_libraries(
    Day3AimSpread
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day3AimSpread
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day3AssetManager
    exercises/day3_asset_manager.cpp
)

target_link_libraries(
    Day3AssetManager
    PRIVATE
    neon_sdl
)

if(MSVC)
    target_compile_options(
        Day3AssetManager
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day4EnemyKind
    exercises/day4_enemy_kind.cpp
)

target_link_libraries(
    Day4EnemyKind
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day4EnemyKind
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day4ShooterMovement
    exercises/day4_shooter_movement.cpp
)

target_link_libraries(
    Day4ShooterMovement
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day4ShooterMovement
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day4WaveComposition
    exercises/day4_wave_composition.cpp
)

target_link_libraries(
    Day4WaveComposition
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day4WaveComposition
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day4ShooterWarning
    exercises/day4_shooter_warning.cpp
)

target_link_libraries(
    Day4ShooterWarning
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day4ShooterWarning
        PRIVATE /W4 /permissive- /utf-8
    )
endif()
add_executable(
    Day4ProjectileFaction
    exercises/day4_projectile_faction.cpp
)

target_link_libraries(
    Day4ProjectileFaction
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day4ProjectileFaction
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day4PlayerInvulnerability
    exercises/day4_player_invulnerability.cpp
)

target_link_libraries(
    Day4PlayerInvulnerability
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day4PlayerInvulnerability
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day5GameEvent
    exercises/day5_game_event.cpp
)

target_link_libraries(
    Day5GameEvent
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day5GameEvent
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day5SimulationEvents
    exercises/day5_simulation_events.cpp
)

target_link_libraries(
    Day5SimulationEvents
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day5SimulationEvents
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day5EntityId
    exercises/day5_entity_id.cpp
)

target_link_libraries(
    Day5EntityId
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day5EntityId
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day5ReplayFrames
    exercises/day5_replay_frames.cpp
)

target_link_libraries(
    Day5ReplayFrames
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day5ReplayFrames
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day5ReplayTape
    exercises/day5_replay_tape.cpp
)

target_link_libraries(
    Day5ReplayTape
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day5ReplayTape
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day5ReloadEvents
    exercises/day5_reload_events.cpp
)

target_link_libraries(
    Day5ReloadEvents
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day5ReloadEvents
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6DetectionEvent
    exercises/day6_detection_event.cpp
)

target_link_libraries(
    Day6DetectionEvent
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6DetectionEvent
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6SpeedDetector
    exercises/day6_speed_detector.cpp
)

target_link_libraries(
    Day6SpeedDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6SpeedDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6TeleportDetector
    exercises/day6_teleport_detector.cpp
)

target_link_libraries(
    Day6TeleportDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6TeleportDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6AutoFireDetector
    exercises/day6_auto_fire_detector.cpp
)

target_link_libraries(
    Day6AutoFireDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6AutoFireDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6HealthDetector
    exercises/day6_health_detector.cpp
)

target_link_libraries(
    Day6HealthDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6HealthDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6DamageDetector
    exercises/day6_damage_detector.cpp
)

target_link_libraries(
    Day6DamageDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6DamageDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6ScoreDetector
    exercises/day6_score_detector.cpp
)

target_link_libraries(
    Day6ScoreDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6ScoreDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6AimAssistDetector
    exercises/day6_aim_assist_detector.cpp
)

target_link_libraries(
    Day6AimAssistDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6AimAssistDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6EspDetector
    exercises/day6_esp_detector.cpp
)

target_link_libraries(
    Day6EspDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6EspDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

add_executable(
    Day6AutoDodgeDetector
    exercises/day6_auto_dodge_detector.cpp
)

target_link_libraries(
    Day6AutoDodgeDetector
    PRIVATE
    neon_core
)

if(MSVC)
    target_compile_options(
        Day6AutoDodgeDetector
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

if(MSVC)
    target_compile_options(
        Day1EntityExercise
        PRIVATE /W4 /permissive- /utf-8
    )

    target_compile_options(
        Day1CorePlayer
        PRIVATE /W4 /permissive- /utf-8
    )

    target_compile_options(
        Day1CoreProjectile
        PRIVATE /W4 /permissive- /utf-8
    )

        target_compile_options(
        Day1CoreEnemy
        PRIVATE /W4 /permissive- /utf-8
    )

        target_compile_options(
        Day1CoreWaveManager
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

target_link_libraries(
    NeonSiege
    PRIVATE
    neon_core
    neon_sdl
    SDL2::SDL2-static
    SDL2_image::SDL2_image-static
)

if(MSVC)
    target_compile_options(NeonSiege PRIVATE /W4 /permissive- /utf-8)
endif()

if(BUILD_TESTING)
    add_test(NAME CoreTypes COMMAND Day1CoreTypes)
    add_test(NAME CoreCollision COMMAND Day1CoreCollision)
    add_test(NAME CorePlayer COMMAND Day1CorePlayer)
    add_test(NAME CoreProjectile COMMAND Day1CoreProjectile)
    add_test(NAME CoreEnemy COMMAND Day1CoreEnemy)
    add_test(NAME CoreWaveManager COMMAND Day1CoreWaveManager)
    add_test(NAME InputCommand COMMAND Day2InputCommand)
    add_test(NAME GameplayConfig COMMAND Day2GameplayConfig)
    add_test(NAME Simulation COMMAND Day2Simulation)
    add_test(NAME Obstacle COMMAND Day3Obstacle)
    add_test(NAME LineOfSight COMMAND Day3LineOfSight)
    add_test(NAME SpawnPlacement COMMAND Day3SpawnPlacement)
    add_test(NAME Weapon COMMAND Day3Weapon)
    add_test(NAME AimSpread COMMAND Day3AimSpread)
    add_test(NAME AssetManager COMMAND Day3AssetManager)
    add_test(NAME EnemyKind COMMAND Day4EnemyKind)
    add_test(NAME ShooterMovement COMMAND Day4ShooterMovement)
    add_test(NAME WaveComposition COMMAND Day4WaveComposition)
    add_test(NAME ShooterWarning COMMAND Day4ShooterWarning)
    add_test(NAME ProjectileFaction COMMAND Day4ProjectileFaction)
    add_test(NAME PlayerInvulnerability COMMAND Day4PlayerInvulnerability)
    add_test(NAME GameEvent COMMAND Day5GameEvent)
    add_test(NAME SimulationEvents COMMAND Day5SimulationEvents)
    add_test(NAME EntityId COMMAND Day5EntityId)
    add_test(NAME ReplayFrames COMMAND Day5ReplayFrames)
    add_test(NAME ReplayTape COMMAND Day5ReplayTape)
    add_test(NAME ReloadEvents COMMAND Day5ReloadEvents)
    add_test(NAME DetectionEvent COMMAND Day6DetectionEvent)
    add_test(NAME SpeedDetector COMMAND Day6SpeedDetector)
    add_test(NAME TeleportDetector COMMAND Day6TeleportDetector)
    add_test(NAME AutoFireDetector COMMAND Day6AutoFireDetector)
    add_test(NAME HealthDetector COMMAND Day6HealthDetector)
    add_test(NAME DamageDetector COMMAND Day6DamageDetector)
    add_test(NAME ScoreDetector COMMAND Day6ScoreDetector)
    add_test(NAME AimAssistDetector COMMAND Day6AimAssistDetector)
    add_test(NAME EspDetector COMMAND Day6EspDetector)
    add_test(NAME AutoDodgeDetector COMMAND Day6AutoDodgeDetector)
    add_test(NAME MapCatalog COMMAND CommercialMapCatalog)
    add_test(NAME AssetCatalog COMMAND CommercialAssetCatalog)
    add_test(NAME UpgradeSystem COMMAND CommercialUpgradeSystem)
    add_test(NAME WaveDifficulty COMMAND CommercialWaveDifficulty)
    add_test(NAME EnemyObstacleDamage COMMAND Day7EnemyObstacleDamage)
    add_test(NAME AutoReload COMMAND Day7AutoReload)
    add_test(NAME SpawnSafety COMMAND Day7SpawnSafety)
    add_test(NAME EnemyBoundary COMMAND Day7EnemyBoundary)
    add_test(NAME EnemyNavigation COMMAND Day7EnemyNavigation)
    add_test(NAME GameOverOverlay COMMAND Day7GameOverOverlay)
    add_test(NAME FrontendStates COMMAND Day7FrontendStates)
    add_test(NAME PresentationEvents COMMAND Day7PresentationEvents)
    add_test(NAME FrontendInput COMMAND Day7FrontendInput)
    add_test(NAME NeonSiegeSmoke COMMAND NeonSiege --smoke-test)
endif()
