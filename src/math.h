#ifndef MATH_H
#define MATH_H

typedef union {
    struct {
        f32 x;
        f32 y;
    };

    struct {
        f32 u;
        f32 v;
    };
} vec2;

static inline vec2
vec2xy(f32 x, f32 y) {
    vec2 result;

    result.x = x;
    result.y = y;

    return result;
}

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
} vec4;

typedef struct {
    vec2 min;
    vec2 max;
} rect2;

static inline rect2
rect2minmax(vec2 min, vec2 max) {
    rect2 result;

    result.min = min;
    result.max = max;

    return result;
}

#endif
