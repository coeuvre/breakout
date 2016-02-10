#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <SDL2/SDL.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#define logerr(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#define count(a) (sizeof(a) / sizeof(*(a)))

#endif
