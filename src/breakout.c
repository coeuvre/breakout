#include "breakout.h"

struct render_context {
    SDL_Texture *texture;
    uint32_t *buf;
    uint32_t width;
    uint32_t height;
};

static void
copy_pixels_to_texture(struct render_context *ctx, SDL_Texture *texture) {
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

    struct render_context ctx;
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
            }
        }

        copy_pixels_to_texture(&ctx, texture);
        SDL_RenderCopy(renderer, texture, 0, 0);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
