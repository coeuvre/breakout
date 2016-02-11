#include "breakout.h"
#include "renderer.c"

#define MAX_ENTITY_COUNT 1024

typedef enum {
    ENTITY_TYPE_BLOCK,
    ENTITY_TYPE_PADDLE,
    ENTITY_TYPE_BALL,
    ENTITY_TYPE_WALL,
} EntityType;

typedef struct {
    EntityType type;
    vec2 pos;
    vec2 size;
    vec2 vel;
} Entity;

typedef struct {
    u32 entity_count;
    Entity entities[MAX_ENTITY_COUNT];

    u32 player_paddle_index;
} GameState;

typedef struct {
    Entity *entity;
    u32 index;
} AddEntityResult;

static AddEntityResult
add_entity(GameState *gamestate, EntityType type, vec2 pos) {
    assert(gamestate->entity_count < count(gamestate->entities));

    u32 index = gamestate->entity_count++;
    Entity *entity = gamestate->entities + index;
    entity->type = type;
    entity->pos = pos;

    AddEntityResult result;
    result.entity = entity;
    result.index = index;
    return result;
}

static AddEntityResult
add_block(GameState *gamestate, rect2 rect) {
    AddEntityResult result = add_entity(gamestate, ENTITY_TYPE_BLOCK, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static AddEntityResult
add_wall(GameState *gamestate, rect2 rect) {
    AddEntityResult result = add_entity(gamestate, ENTITY_TYPE_WALL, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static AddEntityResult
add_paddle(GameState *gamestate, rect2 rect) {
    AddEntityResult result = add_entity(gamestate, ENTITY_TYPE_PADDLE, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    return result;
}

static AddEntityResult
add_ball(GameState *gamestate, rect2 rect, vec2 vel) {
    AddEntityResult result = add_entity(gamestate, ENTITY_TYPE_BALL, get_rect2_cen(rect));
    result.entity->size = get_rect2_size(rect);
    result.entity->vel = vel;
    return result;
}

typedef struct {
    line2 line;
    vec2 normal;
} TestLine2;

static void
move_entity(GameState *gamestate, Entity *entity, f32 dt) {
    vec2 dp = vec2mul(dt, entity->vel);

    for (int iteration = 0; vec2lensq(dp) > 0.0f && iteration < 4; ++iteration) {
        f32 mint = 1.0;
        vec2 targetp = vec2add(entity->pos, dp);
        vec2 normal = vec2zero();
        Entity *hit_entity = 0;

        for (int entity_index = 0; entity_index < gamestate->entity_count; ++entity_index) {
            Entity *test_entity = gamestate->entities + entity_index;

            if (entity == test_entity) {
                continue;
            }

            vec2 minkowski_size = vec2add(entity->size, test_entity->size);
            rect2 bound = rect2censize(vec2zero(), minkowski_size);
            vec2 rel = vec2sub(entity->pos, test_entity->pos);
            ray2 motion = ray2od(rel, dp);
            TestLine2 test_lines[] = {
                // Top
                { line2ab(vec2xy(bound.min.x, bound.max.y), bound.max), vec2xy(0.0f, 1.0f) },
                // Right
                { line2ab(vec2xy(bound.max.x, bound.min.y), bound.max), vec2xy(1.0f, 0.0f) },
                // Left
                { line2ab(bound.min, vec2xy(bound.min.x, bound.max.y)), vec2xy(-1.0f, 0.0f) },
                // Bottom
                { line2ab(bound.min, vec2xy(bound.max.x, bound.min.y)), vec2xy(0.0f, -1.0f) },
            };

            for (int line_index = 0; line_index < count(test_lines); ++line_index) {
                TestLine2 *test_line = test_lines + line_index;
                if (vec2dot(dp, test_line->normal) < 0.0f) {
                    Intersection intersection = ray2_line2_intersection_test(motion, test_line->line);
                    if (intersection.has) {
                        if (intersection.t < mint) {
                            hit_entity = test_entity;
                            mint = intersection.t;
                            normal = test_line->normal;
                        }
                    }
                }
            }
        }

        entity->pos = vec2add(entity->pos, vec2mul(mint, dp));

        dp = vec2sub(targetp, entity->pos);

        if (hit_entity) {
            // dp = dp - 2.0f * dp * normal * normal;
            dp = vec2sub(dp, vec2mul(2.0f, vec2mul(vec2dot(dp, normal), normal)));
            entity->pos = vec2add(entity->pos, dp);

            // entity->vel = entity->vel - 2.0f * entity->vel * normal * normal;
            entity->vel = vec2sub(
                entity->vel,
                vec2mul(2.0f, vec2mul(vec2dot(entity->vel, normal), normal))
            );
        }
    }
}

static void
init(GameState *gamestate) {
    // Build blocks
    {
        vec2 margin = vec2xy(100.0f, 300.0f);
        vec2 size = vec2xy(50.0f, 20.0f);
        vec2 min = margin;
        f32 padding = 10.0f;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 10; ++x) {
                add_block(gamestate, rect2minsize(min, size));
                min.x += size.x + padding;
            }
            min.x = margin.x;
            min.y += size.y + padding;
        }
    }

    // Build walls
    {
        // left
        add_wall(gamestate, rect2minsize(vec2xy(0.0f, 0.0f), vec2xy(15.0f, 600.0f)));
        // top
        add_wall(gamestate, rect2minsize(vec2xy(0.0f, 600.0f - 15.0f), vec2xy(800.0f, 15.0f)));
        // right
        add_wall(gamestate, rect2minsize(vec2xy(800.0f - 15.0f, 0.0f), vec2xy(15.0f, 600.0f)));
        // down
        add_wall(gamestate, rect2minsize(vec2xy(0.0f, -15.0f), vec2xy(800.0f, 15.0f)));
    }

    gamestate->player_paddle_index = add_paddle(
        gamestate,
        rect2censize(vec2xy(400.0f, 35.0f), vec2xy(100.0f, 30.0f))
    ).index;

    add_ball(gamestate, rect2censize(vec2xy(400.0f, 150.0f), vec2xy(15.0f, 15.0f)), vec2xy(150.0f, 150.0f));
}

static void
handle_event(GameState *gamestate, SDL_Event *e) {
    switch (e->type) {
        case SDL_MOUSEMOTION: {
            Entity *paddle = gamestate->entities + gamestate->player_paddle_index;
            paddle->pos.x = e->motion.x;
        } break;
        default: break;
    }
}

static void
update_and_render(GameState *gamestate, RenderContext *ctx, f32 dt) {
    for (int i = 0; i < gamestate->entity_count; ++i) {
        Entity *entity = gamestate->entities + i;

        move_entity(gamestate, entity, dt);

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

    i32 window_w = 800;
    i32 window_h = 600;

    // On Apple's OS X you must set the NSHighResolutionCapable
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

    RenderContext ctx;
    ctx.renderer = renderer;
    ctx.texture = texture;
    ctx.buf = buffer;
    ctx.width = window_w;
    ctx.height = window_h;

    GameState gamestate = {};
    init(&gamestate);

    f32 dt = 1.0f / 60.0f;
    //u32 target_frametime = dt * 1000.0f;

    i32 quit = 0;
    while (!quit) {
        //u32 frame_begin = SDL_GetTicks();

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

            handle_event(&gamestate, &e);
        }

        update_and_render(&gamestate, &ctx, dt);

        //u32 frametime = SDL_GetTicks() - frame_begin;
        //printf("%u\n", frametime);

        render_to_screen(&ctx);

#if 0
        frametime = SDL_GetTicks() - frame_begin;
        if (target_frametime > frametime) {
            SDL_Delay(target_frametime - frametime);
        }
#endif
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
