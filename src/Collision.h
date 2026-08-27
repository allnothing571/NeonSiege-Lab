#pragma once

#include <SDL.h>

inline bool intersects(
    const SDL_FRect& first,
    const SDL_FRect& second) {
    return
        first.x < second.x + second.w &&
        first.x + first.w > second.x &&
        first.y < second.y + second.h &&
        first.y + first.h > second.y;
}