#include "breakout.h"

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
    vec2 result = {x, y};
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

static inline vec4
rgba(f32 r, f32 g, f32 b, f32 a) {
    vec4 result;

    result.r = r;
    result.g = g;
    result.b = b;
    result.a = a;

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

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    u32 *buf;
    u32 width;
    u32 height;
} render_context;

static void
copy_pixels_to_texture(render_context *ctx) {
    void *pixels;
    int pitch;
    SDL_LockTexture(ctx->texture, 0, &pixels, &pitch);

    u32 *src = ctx->buf;
    u8 *row = pixels;
    for (u32 y = 0; y < ctx->height; ++y) {
        u32 *dst = (u32 *) row;
        for (u32 x = 0; x < ctx->width; ++x) {
            *dst++ = *src++;
        }
        row += pitch;
    }

    SDL_UnlockTexture(ctx->texture);
}

static inline u32
rgba_to_uint32(vec4 color) {
    u8 r = (u8) (color.r * 255.0f);
    u8 g = (u8) (color.g * 255.0f);
    u8 b = (u8) (color.b * 255.0f);
    u8 a = (u8) (color.a * 255.0f);

    // 0xRRGGBBAA
    u32 result = (r << 24) | (g << 16) | (b << 8) | (a << 0);
    return result;
}

static void
render_rect(render_context *ctx, rect2 rect, vec4 rgba) {
    u32 minx = (u8) (rect.min.x);
    u32 miny = (u8) (rect.min.y);
    u32 maxx = (u8) (rect.max.x);
    u32 maxy = (u8) (rect.max.y);

    u32 color = rgba_to_uint32(rgba);
    u32 *row = ctx->buf + (miny * ctx->width) + minx;
    for (u32 y = miny; y < maxy; ++y) {
        u32 *pixel = row;
        for (u32 x = minx; x < maxx; ++x) {
            *pixel++ = color;
        }
        row += ctx->width;
    }
}

static void
render_to_screen(render_context *ctx) {
    copy_pixels_to_texture(ctx);
    memset(ctx->buf, 0, sizeof(*ctx->buf) * ctx->width * ctx->height);

    SDL_RenderCopy(ctx->renderer, ctx->texture, 0, 0);
    SDL_RenderPresent(ctx->renderer);
}

int
main(void) {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logerr("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    u32 window_w = 800;
    u32 window_h = 600;

    // NOTE: On Apple's OS X you must set the NSHighResolutionCapable
    // Info.plist property to YES, otherwise you will not receive a
    // High DPI OpenGL canvas.
    SDL_Window *window = SDL_CreateWindow("Breakout",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          window_w, window_h,
                                          SDL_WINDOW_OPENGL);
    if (!window) {
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
    u32 buffer[window_w * window_h];
    memset(buffer, 0, sizeof(buffer));

    render_context ctx;
    ctx.renderer = renderer;
    ctx.texture = texture;
    ctx.buf = buffer;
    ctx.width = window_w;
    ctx.height = window_h;

    i32 quit = 0;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = 1;
                } break;

                case SDL_KEYDOWN: {
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                    }
                } break;

                default: break;
            }
        }

        render_rect(&ctx, rect2minmax(vec2xy(0, 0), vec2xy(100, 100)), rgba(1.0f, 1.0f, 1.0f, 1.0f));

        render_to_screen(&ctx);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
