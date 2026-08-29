cmake_minimum_required(VERSION 3.21)

project(NeonSiege VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

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

add_library(neon_core INTERFACE)

target_include_directories(
    neon_core
    INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(
    neon_core
    INTERFACE
    src/core/Types.h
    src/core/Collision.h
    src/core/Player.h
    src/core/Projectile.h
    src/core/Enemy.h
    src/core/WaveManager.h
)

add_library(
    neon_sdl
    STATIC
    src/sdl/SdlGameRenderer.h
    src/sdl/SdlGameRenderer.cpp
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
src/Collision.h
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
