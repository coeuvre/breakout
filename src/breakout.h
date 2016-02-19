#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <SDL2/SDL.h>
#undef main

#include <assert.h>
#include <string.h>

#define logerr(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#define count(a) (sizeof(a) / sizeof(*(a)))

#include "types.h"
#include "math.h"
#include "renderer.h"

#endif
