cmake_minimum_required(VERSION 3.21)

project(NeonSiege VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2)

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
    src/core/GameSnapshot.h
    src/core/Collision.h
    src/core/Obstacle.h
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
    src/sdl/SdlInput.h
    src/sdl/SdlInput.cpp
)

target_include_directories(
    neon_sdl
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    neon_sdl
    PUBLIC neon_core
    PRIVATE SDL2::SDL2-static
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
src/SaveData.h
src/SaveData.cpp)

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
    add_test(NAME NeonSiegeSmoke COMMAND NeonSiege --smoke-test)
endif()