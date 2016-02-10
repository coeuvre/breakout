#include "breakout.h"

typedef union {
    struct {
        float x;
        float y;
    };

    struct {
        float u;
        float v;
    };
} Vec2;

static Vec2
vec2(float x, float y) {
    Vec2 result;

    result.x = x;
    result.y = y;

    return result;
}

typedef union {
    struct {
        float x;
        float y;
        float z;
        float w;
    };

    struct {
        float r;
        float g;
        float b;
        float a;
    };
} Vec4;

static Vec4
rgba(float r, float g, float b, float a) {
    Vec4 result;

    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

    return result;
}

typedef struct {
    Vec2 min;
    Vec2 max;
} Rect2;

static Rect2
rect2_with_min_max(Vec2 min, Vec2 max) {
    Rect2 result;

    result.min = min;
    result.max = max;

    return result;
}

typedef struct {
    SDL_Texture *texture;
    uint32_t *buf;
    uint32_t width;
    uint32_t height;
} RenderContext;

static void
copy_pixels_to_texture(RenderContext *ctx, SDL_Texture *texture) {
    void *pixels;
    int pitch;
    SDL_LockTexture(ctx->texture, 0, &pixels, &pitch);

    uint32_t *src = ctx->buf;
    uint8_t *row = pixels;
    for (uint32_t y = 0; y < ctx->height; ++y) {
        uint32_t *dst = (uint32_t *) row;
        for (uint32_t x = 0; x < ctx->width; ++x) {
            *dst++ = *src++;
        }
        row += pitch;
    }

    SDL_UnlockTexture(texture);
}

static uint32_t
rgba_to_uint32(Vec4 color) {
    uint8_t r = (uint8_t) (color.r * 255.0f);
    uint8_t g = (uint8_t) (color.g * 255.0f);
    uint8_t b = (uint8_t) (color.b * 255.0f);
    uint8_t a = (uint8_t) (color.a * 255.0f);

    // 0xRRGGBBAA
    uint32_t result = (r << 24) | (g << 16) | (b << 8) | (a << 0);
    return result;
}

static void
draw_rect(RenderContext *ctx, Rect2 rect, Vec4 rgba) {
    uint32_t minx = (uint8_t) (rect.min.x);
    uint32_t miny = (uint8_t) (rect.min.y);
    uint32_t maxx = (uint8_t) (rect.max.x);
    uint32_t maxy = (uint8_t) (rect.max.y);

    uint32_t color = rgba_to_uint32(rgba);
    uint32_t *row = ctx->buf + (miny * ctx->width) + minx;
    for (uint32_t y = miny; y < maxy; ++y) {
        uint32_t *pixel = row;
        for (uint32_t x = minx; x < maxx; ++x) {
            *pixel++ = color;
        }
        row += ctx->width;
    }
}

int
main(void) {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logerr("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    uint32_t window_w = 800;
    uint32_t window_h = 600;

    // NOTE: On Apple's OS X you must set the NSHighResolutionCapable
    // Info.plist property to YES, otherwise you will not receive a
    // High DPI OpenGL canvas.
    SDL_Window *window = SDL_CreateWindow("Breakout",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          window_w, window_h,
                                          SDL_WINDOW_OPENGL);
    if (window == NULL) {
        logerr("Failed to create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        logerr("Failed to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             window_w, window_h);
    uint32_t buffer[window_w * window_h];
    memset(buffer, 0, sizeof(buffer));

    RenderContext ctx;
    ctx.texture = texture;
    ctx.buf = buffer;
    ctx.width = window_w;
    ctx.height = window_h;

    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = true;
                } break;

                case SDL_KEYDOWN: {
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        quit = true;
                    }
                } break;

                default: break;
            }
        }

        draw_rect(&ctx, rect2_with_min_max(vec2(0, 0), vec2(100, 100)), rgba(1.0f, 1.0f, 1.0f, 1.0f));

        copy_pixels_to_texture(&ctx, texture);
        SDL_RenderCopy(renderer, texture, 0, 0);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
