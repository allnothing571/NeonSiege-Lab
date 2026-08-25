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

add_executable(NeonSiege src/main.cpp)

add_executable(
    Day1EntityExercise
    exercises/day1_entity_manager.cpp
)

if(MSVC)
    target_compile_options(
        Day1EntityExercise
        PRIVATE /W4 /permissive- /utf-8
    )
endif()

target_link_libraries(NeonSiege PRIVATE SDL2::SDL2-static)

if(MSVC)
    target_compile_options(NeonSiege PRIVATE /W4 /permissive- /utf-8)
endif()

