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
v2(f32 x, f32 y) {
    vec2 result;

    result.x = x;
    result.y = y;

    return result;
}

static inline vec2
v2zero() {
    return v2(0.0f, 0.0f);
}

static inline vec2
v2add(vec2 l, vec2 r) {
    vec2 result;

    result.x = l.x + r.x;
    result.y = l.y + r.y;

    return result;
}

static inline vec2
v2sub(vec2 l, vec2 r) {
    vec2 result;

    result.x = l.x - r.x;
    result.y = l.y - r.y;

    return result;
}

static inline vec2
v2mul(f32 l, vec2 r) {
    vec2 result;

    result.x = l * r.x;
    result.y = l * r.y;

    return result;
}

static inline f32
v2dot(vec2 l, vec2 r) {
    f32 result = l.x * r.x + l.y * r.y;
    return result;
}

static inline f32
v2lensq(vec2 v) {
    f32 result = v2dot(v, v);
    return result;
}

typedef struct {
    i32 has;
    f32 x;
    f32 y;
} linear_system2_solution;

static inline linear_system2_solution
solve_linear_system2(vec2 a, vec2 b, vec2 c) {
    linear_system2_solution result = {};

    f32 d = a.x * b.y - a.y * b.x;

    if (d != 0.0f) {
        result.has = 1;
        vec2 solution = v2((c.x * b.y - c.y * b.x) / d,
                           (a.x * c.y - a.y * c.x) / d);
        result.x = solution.x;
        result.y = solution.y;
    }

    return result;
}

typedef struct {
    vec2 a;
    vec2 b;
} line2;

static inline line2
line2ab(vec2 a, vec2 b) {
    line2 result;

    result.a = a;
    result.b = b;

    return result;
}

typedef struct {
    vec2 o;
    vec2 d;
} ray2;

typedef struct {
    i32 has;
    f32 t;
} intersection;

static inline intersection
ray2_line2_intersection_test(ray2 ray, line2 line) {
    intersection result = {};

    vec2 a = ray.d;
    vec2 b = v2sub(line.a, line.b);
    vec2 c = v2sub(line.a, ray.o);

    linear_system2_solution solution = solve_linear_system2(a, b, c);
    if (solution.has && solution.y >= 0 && solution.y <= 1.0 &&
        solution.x >= 0.0)
    {
        result.has = 1;
        result.t = solution.x;
    }

    return result;
}

static inline ray2
ray2od(vec2 o, vec2 d) {
    ray2 result;

    result.o = o;
    result.d = d;

    return result;
}

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
    result.max = v2add(min, size);

    return result;
}

static inline rect2
rect2censize(vec2 cen, vec2 size) {
    rect2 result;

    vec2 halfsize = v2mul(0.5f, size);
    result.min = v2sub(cen, halfsize);
    result.max = v2add(cen, halfsize);

    return result;
}

static inline vec2
getrect2cen(rect2 rect) {
    // rect.min + 0.5f * (rect.max - rect.min)
    vec2 result = v2add(rect.min, v2mul(0.5f, v2sub(rect.max, rect.min)));
    return result;
}

static inline vec2
getrect2size(rect2 rect) {
    vec2 result = v2sub(rect.max, rect.min);
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

#endif
