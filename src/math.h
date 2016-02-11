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

static inline vec2
vec2add(vec2 l, vec2 r) {
    vec2 result;

    result.x = l.x + r.x;
    result.y = l.y + r.y;

    return result;
}

static inline vec2
vec2sub(vec2 l, vec2 r) {
    vec2 result;

    result.x = l.x - r.x;
    result.y = l.y - r.y;

    return result;
}

static inline vec2
vec2mul(f32 l, vec2 r) {
    vec2 result;

    result.x = l * r.x;
    result.y = l * r.y;

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

static inline rect2
rect2minsize(vec2 min, vec2 size) {
    rect2 result;

    result.min = min;
    result.max = vec2add(min, size);

    return result;
}

static inline rect2
rect2censize(vec2 cen, vec2 size) {
    rect2 result;

    vec2 halfsize = vec2mul(0.5f, size);
    result.min = vec2sub(cen, halfsize);
    result.max = vec2add(cen, halfsize);

    return result;
}

static inline vec2
get_rect2_cen(rect2 rect) {
    // rect.min + 0.5f * (rect.max - rect.min)
    vec2 result = vec2add(rect.min, vec2mul(0.5f, vec2sub(rect.max, rect.min)));
    return result;
}

static inline vec2
get_rect2_size(rect2 rect) {
    vec2 result = vec2sub(rect.max, rect.min);
    return result;
}

#endif
