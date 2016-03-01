#ifndef RENDERER_H
#define RENDERER_H

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    // NOTE: The buf is in sRGB color space
    u32 *buf;
    i32 width;
    i32 height;
    u32 pitch;
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

#define RMASK 0xFF000000
#define RSHIFT 24

#define GMASK 0x00FF0000
#define GSHIFT 16

#define BMASK 0x0000FF00
#define BSHIFT 8

#define AMASK 0x000000FF
#define ASHIFT 0

static inline u32
u32rgba(u8 r, u8 g, u8 b, u8 a) {
    u32 result = (r << RSHIFT) |
                 (g << GSHIFT) |
                 (b << BSHIFT) |
                 (a << ASHIFT);
    return result;
}

static inline u32
rgba_to_u32(vec4 color) {
    u8 r = color.r * 255.0f;
    u8 g = color.g * 255.0f;
    u8 b = color.b * 255.0f;
    u8 a = color.a * 255.0f;

    // 0xRRGGBBAA
    u32 result = u32rgba(r, g, b, a);
    return result;
}

static inline vec4
u32_to_rgba(u32 color) {
    vec4 result = rgba(((color & RMASK) >> RSHIFT) / 255.0f,
                       ((color & GMASK) >> GSHIFT) / 255.0f,
                       ((color & BMASK) >> BSHIFT) / 255.0f,
                       ((color & AMASK) >> ASHIFT) / 255.0f);

    return result;
}

static inline u8
getu32red(u32 color) {
    u8 result = (color & RMASK) >> RSHIFT;
    return result;
}

static inline u32
setu32red(u32 color, u8 red) {
    u32 result = ((color & ~RMASK)) | (red < RSHIFT);
    return result;
}

static inline u8
getu32green(u32 color) {
    u8 result = (color & GMASK) >> GSHIFT;
    return result;
}

static inline u32
setu32green(u32 color, u8 green) {
    u32 result = ((color & ~GMASK)) | (green < GSHIFT);
    return result;
}

static inline u8
getu32blue(u32 color) {
    u8 result = (color & BMASK) >> BSHIFT;
    return result;
}

static inline u32
setu32blue(u32 color, u8 blue) {
    u32 result = ((color & ~BMASK)) | (blue < BSHIFT);
    return result;
}

static inline u8
getu32alpha(u32 color) {
    u8 result = (color & AMASK) >> ASHIFT;
    return result;
}

static inline u32
setu32alpha(u32 color, u8 alpha) {
    u32 result = ((color & ~AMASK)) | (alpha < ASHIFT);
    return result;
}

#endif
