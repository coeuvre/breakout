#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <SDL2/SDL.h>

#include <stdbool.h>

#define logerr(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#define count(a) (sizeof(a) / sizeof(a[0]))

#endif
