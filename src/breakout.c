#include "breakout.h"
#include "renderer.c"

#define MAX_ENTITY_COUNT 1024

typedef enum {
    ENTITY_TYPE_BLOCK,
    ENTITY_TYPE_PADDLE,
    ENTITY_TYPE_BALL,
    ENTITY_TYPE_WALL,
} entity_type;

typedef struct {
    entity_type type;
    vec2 pos;
    vec2 size;
    vec2 vel;
} entity;

typedef struct {
    u32 entity_count;
    entity entities[MAX_ENTITY_COUNT];

    u32 player_paddle_index;
} game_state;

typedef struct {
    entity *entity;
    u32 index;
} add_entity_result;

static add_entity_result
add_entity(game_state *game_state, entity_type type, vec2 pos) {
    assert(game_state->entity_count < count(game_state->entities));

    u32 index = game_state->entity_count++;
    entity *entity = game_state->entities + index;
    entity->type = type;
    entity->pos = pos;

    add_entity_result result;
    result.entity = entity;
    result.index = index;
    return result;
}

static add_entity_result
add_block(game_state *game_state, rect2 rect) {
    add_entity_result result = add_entity(game_state, ENTITY_TYPE_BLOCK, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static add_entity_result
add_wall(game_state *game_state, rect2 rect) {
    add_entity_result result = add_entity(game_state, ENTITY_TYPE_WALL, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static add_entity_result
add_paddle(game_state *game_state, rect2 rect) {
    add_entity_result result = add_entity(game_state, ENTITY_TYPE_PADDLE, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static add_entity_result
add_ball(game_state *game_state, rect2 rect) {
    add_entity_result result = add_entity(game_state, ENTITY_TYPE_BALL, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static void
init(game_state *game_state) {
    // Build blocks
    {
        vec2 margin = vec2xy(100.0f, 300.0f);
        vec2 size = vec2xy(50.0f, 20.0f);
        vec2 min = margin;
        f32 padding = 10.0f;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 10; ++x) {
                add_block(game_state, rect2minsize(min, size));
                min.x += size.x + padding;
            }
            min.x = margin.x;
            min.y += size.y + padding;
        }
    }

    // Build walls
    {
        // left
        add_wall(game_state, rect2minsize(vec2xy(0.0f, 0.0f), vec2xy(15.0f, 600.0f)));
        // top
        add_wall(game_state, rect2minsize(vec2xy(0.0f, 600.0f - 15.0f), vec2xy(800.0f, 15.0f)));
        // right
        add_wall(game_state, rect2minsize(vec2xy(800.0f - 15.0f, 0.0f), vec2xy(15.0f, 600.0f)));
        // down
        add_wall(game_state, rect2minsize(vec2xy(0.0f, -15.0f), vec2xy(800.0f, 15.0f)));
    }

    game_state->player_paddle_index = add_paddle(
        game_state,
        rect2censize(vec2xy(400.0f, 35.0f), vec2xy(100.0f, 30.0f))
    ).index;

    add_ball(game_state, rect2censize(vec2xy(400.0f, 150.0f), vec2xy(15.0f, 15.0f)));
}

static void
handle_event(game_state *game_state, SDL_Event e) {
    switch (e.type) {
        case SDL_MOUSEMOTION: {
            entity *paddle = game_state->entities + game_state->player_paddle_index;
            paddle->pos.x = e.motion.x;
        } break;
        default: break;
    }
}

static void
update(game_state *game_state, f32 dt) {
}

static void
render(game_state *game_state, render_context *ctx) {
    for (int i = 0; i < game_state->entity_count; ++i) {
        entity *entity = game_state->entities + i;

        render_rect(ctx, rect2censize(entity->pos, entity->size), rgba(1.0f, 1.0f, 1.0f, 1.0f));
    }
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

    game_state game_state = {};
    init(&game_state);

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

            handle_event(&game_state, e);
        }

        update(&game_state, dt);

        render(&game_state, &ctx);

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
