#include "breakout.h"
#include "renderer.c"

static void
update(f32 dt) {
}

static void
render(render_context *ctx) {
    render_rect(ctx, rect2minmax(vec2xy(0, 0), vec2xy(100, 100)), rgba(1.0f, 1.0f, 1.0f, 1.0f));
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

    f32 dt = 1.0f / 60.0f;
    u32 target_frametime = dt * 1000.0f;

    i32 quit = 0;
    while (!quit) {
        u32 frame_begin = SDL_GetTicks();

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

        update(dt);

        render(&ctx);

        u32 frametime = SDL_GetTicks() - frame_begin;
        //printf("%u\n", frametime);

        render_to_screen(&ctx);

        frametime = SDL_GetTicks() - frame_begin;
        if (target_frametime > frametime) {
            SDL_Delay(target_frametime - frametime);
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
