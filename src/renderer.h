#ifndef RENDERER_H
#define RENDERER_H

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    u32 *buf;
    i32 width;
    i32 height;
} render_context;

static inline vec4
rgba(f32 r, f32 g, f32 b, f32 a) {
    vec4 result;

    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

    return result;
}

#endif
