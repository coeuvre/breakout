#include "breakout.h"
#include "renderer.c"

#define MAX_ENTITY_COUNT 1024

typedef enum {
    ENTITY_TYPE_BLOCK,
    ENTITY_TYPE_PADDLE,
    ENTITY_TYPE_BALL,
    ENTITY_TYPE_BALL_TAIL,
    ENTITY_TYPE_WALL,
} entity_type;

enum {
    ENTITY_FLAG_REMOVED = (1 << 0),
    ENTITY_FLAG_COLLIDE = (1 << 1),
};

typedef struct {
    u32 index;
    entity_type type;
    u32 flags;
    vec2 pos;
    vec2 size;
    vec2 vel;
} entity;

typedef struct {
    u32 entity_count;
    entity entities[MAX_ENTITY_COUNT];

    u32 free_entity_index_count;
    u32 free_entity_indices[MAX_ENTITY_COUNT];

    entity *player_paddle;
} game_state;

static void
init_game_state(game_state *gs) {
    // NOTE: The entity index 0 is considerd null entity
    for (u32 free_entity_index = MAX_ENTITY_COUNT - 1; free_entity_index != 0; --free_entity_index) {
        gs->free_entity_indices[gs->free_entity_index_count++] = free_entity_index;
    }
}

static u32
next_free_entity_index(game_state *gs) {
    assert(gs->entity_count < count(gs->entities));
    assert(gs->free_entity_index_count > 0);

    u32 index = gs->free_entity_indices[--gs->free_entity_index_count];
    if (index >= gs->entity_count) {
        gs->entity_count = index + 1;
    }
    return index;
}

static int
is_entity_set(entity *e, u32 flags) {
    return e->flags & flags;
}

static void
set_entity(entity *e, u32 flags) {
    e->flags |= flags;
}

static entity *
add_entity(game_state *gs, entity_type type, vec2 pos) {
    u32 index = next_free_entity_index(gs);

    entity *e = gs->entities + index;
    *e = (entity) {};

    e->index = index;
    e->type = type;
    e->pos = pos;
    return e;
}

static void
remove_entity(game_state *gs, entity *e) {
    assert(gs->free_entity_index_count < MAX_ENTITY_COUNT);
    gs->free_entity_indices[gs->free_entity_index_count++] = e->index;

    set_entity(e, ENTITY_FLAG_REMOVED);
}

static entity *
add_block(game_state *gs, rect2 rect) {
    entity *e = add_entity(gs, ENTITY_TYPE_BLOCK, getrect2cen(rect));
    e->size = getrect2size(rect);
    set_entity(e, ENTITY_FLAG_COLLIDE);
    return e;
}

static entity *
add_wall(game_state *gs, rect2 rect) {
    entity *e = add_entity(gs, ENTITY_TYPE_WALL, getrect2cen(rect));
    e->size = getrect2size(rect);
    set_entity(e, ENTITY_FLAG_COLLIDE);
    return e;
}

static entity *
add_paddle(game_state *gs, rect2 rect) {
    entity *e = add_entity(gs, ENTITY_TYPE_PADDLE, getrect2cen(rect));
    e->size = getrect2size(rect);
    set_entity(e, ENTITY_FLAG_COLLIDE);
    return e;
}

static entity *
add_ball(game_state *gs, rect2 rect, vec2 vel) {
    entity *e = add_entity(gs, ENTITY_TYPE_BALL, getrect2cen(rect));
    e->size = getrect2size(rect);
    e->vel = vel;
    set_entity(e, ENTITY_FLAG_COLLIDE);
    return e;
}

static entity *
add_ball_tail(game_state *gs, entity *ball) {
    entity *e = add_entity(gs, ENTITY_TYPE_BALL_TAIL, ball->pos);
    e->size = v2mul(0.6, ball->size);
    e->vel = v2zero();
    return e;
}

typedef struct {
    line2 line;
    vec2 normal;
} test_line;

static void
move_entity(game_state *gs, entity *mover, f32 dt) {
    vec2 dp = v2mul(dt, mover->vel);

    for (int iteration = 0; getv2lensq(dp) > 0.0f && iteration < 4; ++iteration) {
        f32 mint = 1.0;
        vec2 targetp = v2add(mover->pos, dp);
        vec2 normal = v2zero();
        entity *hit_entity = 0;
        u32 hit_entity_index = 0;

        for (u32 entity_index = 0; entity_index < gs->entity_count; ++entity_index) {
            entity *test_entity = gs->entities + entity_index;

            if (mover == test_entity || is_entity_set(test_entity, ENTITY_FLAG_REMOVED) ||
                !is_entity_set(test_entity, ENTITY_FLAG_COLLIDE))
            {
                continue;
            }

            vec2 minkowski_size = v2add(mover->size, test_entity->size);
            rect2 bound = rect2censize(v2zero(), minkowski_size);
            vec2 rel = v2sub(mover->pos, test_entity->pos);
            ray2 motion = ray2od(rel, dp);
            test_line test_lines[] = {
                // Top
                { line2ab(v2(bound.min.x, bound.max.y), bound.max), v2(0.0f, 1.0f) },
                // Right
                { line2ab(v2(bound.max.x, bound.min.y), bound.max), v2(1.0f, 0.0f) },
                // Left
                { line2ab(bound.min, v2(bound.min.x, bound.max.y)), v2(-1.0f, 0.0f) },
                // Bottom
                { line2ab(bound.min, v2(bound.max.x, bound.min.y)), v2(0.0f, -1.0f) },
            };

            for (u32 line_index = 0; line_index < count(test_lines); ++line_index) {
                test_line *test_line = test_lines + line_index;
                if (v2dot(dp, test_line->normal) < 0.0f) {
                    intersection intersection = ray2_line2_intersection_test(motion, test_line->line);
                    if (intersection.has) {
                        if (intersection.t < mint) {
                            hit_entity = test_entity;
                            hit_entity_index = entity_index;
                            mint = intersection.t;
                            normal = test_line->normal;
                        }
                    }
                }
            }
        }

        mover->pos = v2add(mover->pos, v2mul(mint, dp));

        dp = v2sub(targetp, mover->pos);

        if (hit_entity) {
            // NOTE: Reflect the remaining motion
            //
            // dp = dp - 2.0f * dp * normal * normal;
            dp = v2sub(dp, v2mul(2.0f, v2mul(v2dot(dp, normal), normal)));

            // NOTE: Reflect the velocity
            //
            // mover->vel = mover->vel - 2.0f * mover->vel * normal * normal;
            mover->vel = v2sub(
                mover->vel,
                v2mul(2.0f, v2mul(v2dot(mover->vel, normal), normal))
            );

            if (hit_entity->type == ENTITY_TYPE_BLOCK) {
                remove_entity(gs, hit_entity);
            }
        }
    }
}

static void
init(game_state *gs) {
    // Build blocks
    {
        vec2 margin = v2(100.0f, 300.0f);
        vec2 size = v2(50.0f, 20.0f);
        vec2 min = margin;
        f32 padding = 10.0f;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 10; ++x) {
                add_block(gs, rect2minsize(min, size));
                min.x += size.x + padding;
            }
            min.x = margin.x;
            min.y += size.y + padding;
        }
    }

    // Build walls
    {
        // left
        add_wall(gs, rect2minsize(v2(0.0f, 0.0f), v2(15.0f, 600.0f)));
        // top
        add_wall(gs, rect2minsize(v2(0.0f, 600.0f - 15.0f), v2(800.0f, 15.0f)));
        // right
        add_wall(gs, rect2minsize(v2(800.0f - 15.0f, 0.0f), v2(15.0f, 600.0f)));
        // down
        add_wall(gs, rect2minsize(v2(0.0f, -15.0f), v2(800.0f, 15.0f)));
    }

    gs->player_paddle = add_paddle(
        gs, rect2censize(v2(400.0f, 35.0f), v2(100.0f, 30.0f))
    );

    add_ball(gs, rect2censize(v2(400.0f, 150.0f), v2(15.0f, 15.0f)), v2(200.0f, 200.0f));
}

static void
handle_event(game_state *gs, SDL_Event *e) {
    switch (e->type) {
        case SDL_MOUSEMOTION: {
            gs->player_paddle->pos.x = e->motion.x;
        } break;
        default: break;
    }
}

static void
update_and_render(game_state *gs, render_context *ctx, f32 dt) {
    for (u32 i = 0; i < gs->entity_count; ++i) {
        entity *e = gs->entities + i;

        if (is_entity_set(e, ENTITY_FLAG_REMOVED)) {
            continue;
        }

        switch (e->type) {
            case ENTITY_TYPE_BALL: {
                add_ball_tail(gs, e);
                move_entity(gs, e, dt);
            } break;

            case ENTITY_TYPE_BALL_TAIL: {
                e->size = v2sub(e->size, v2(0.05f, 0.05f));
                if (getv2lensq(e->size) < 16.0f) {
                    remove_entity(gs, e);
                }
            } break;

            default: break;
        }

        render_rect(ctx, rect2censize(e->pos, e->size), rgba(1.0f, 1.0f, 1.0f, 1.0f));
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

    u32 *buffer = calloc(window_w * window_h, sizeof(u32));

    render_context ctx;
    ctx.renderer = renderer;
    ctx.texture = texture;
    ctx.buf = buffer;
    ctx.width = window_w;
    ctx.height = window_h;

    game_state gs = {};
    init_game_state(&gs);

    init(&gs);

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

            handle_event(&gs, &e);
        }

        update_and_render(&gs, &ctx, dt);

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
