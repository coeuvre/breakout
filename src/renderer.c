#define GAMMA 2.2f

static u32
u32_linear_to_srgb(u32 color) {
    static u8 gamma_correction_table[256];
    static u32 is_gamma_correction_table_initialized;

    if (!is_gamma_correction_table_initialized) {
        f32 inv_gamma = 1.0f / GAMMA;

        for (int i = 0; i < 256; ++i) {
            f32 c = i / 255.0f;
            c = powf(c, inv_gamma);
            gamma_correction_table[i] = (c * 255.0f + 0.5f);
        }

        is_gamma_correction_table_initialized = 1;
    }

    u8 r = getu32red(color);
    u8 g = getu32green(color);
    u8 b = getu32blue(color);
    u8 a = getu32alpha(color);

    u32 result = u32rgba(gamma_correction_table[r],
                         gamma_correction_table[g],
                         gamma_correction_table[b],
                         a);

    return result;
}

static u32
u32_srgb_to_linear(u32 color) {
    static u8 gamma_table[256];
    static u32 is_gamma_table_initialized;

    if (!is_gamma_table_initialized) {
        f32 gamma = GAMMA;

        for (int i = 0; i < 256; ++i) {
            f32 c = i / 255.0f;
            c = powf(c, gamma);
            gamma_table[i] = (c * 255.0f + 0.5f);
        }

        is_gamma_table_initialized = 1;
    }

    u8 r = getu32red(color);
    u8 g = getu32green(color);
    u8 b = getu32blue(color);
    u8 a = getu32alpha(color);

    u32 result = u32rgba(gamma_table[r],
                         gamma_table[g],
                         gamma_table[b],
                         a);

    return result;
}

static void
copy_pixels_to_texture(render_context *ctx) {
    SDL_UpdateTexture(ctx->texture, 0, ctx->buf, ctx->width * 4);
}

static void
render_rect(render_context *ctx, rect2 rect, vec4 rgba) {
    i32 minx = (i32)rect.min.x;
    i32 miny = (i32)rect.min.y;
    i32 maxx = (i32)rect.max.x;
    i32 maxy = (i32)rect.max.y;

    if (minx < 0) { minx = 0; }
    if (maxx >= ctx->width) { maxx = ctx->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= ctx->height) { maxy = ctx->height; }

    u32 color = rgba_to_u32(rgba);
    u32 *row = ctx->buf + (ctx->height - 1 - miny) * ctx->width + minx;
    for (i32 y = miny; y < maxy; ++y) {
        u32 *pixel = row;
        for (i32 x = minx; x < maxx; ++x) {
            *pixel++ = color;
        }
        row -= ctx->width;
    }
}

static void
render_gradient_rect(render_context *ctx, rect2 rect) {
    i32 minx = (i32) (rect.min.x);
    i32 miny = (i32) (rect.min.y);
    i32 maxx = (i32) (rect.max.x);
    i32 maxy = (i32) (rect.max.y);

    if (minx < 0) { minx = 0; }
    if (maxx >= ctx->width) { maxx = ctx->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= ctx->height) { maxy = ctx->height; }

    u32 *row = ctx->buf + (ctx->height - 1 - miny) * ctx->width + minx;
    i32 size = maxx - minx;
    for (i32 y = miny; y < maxy; ++y) {
        u32 *pixel = row;
        for (i32 x = minx; x < maxx; ++x) {
            f32 intensity = (x - minx) * 1.0f / size;
            u32 color = rgba_to_u32(rgba(intensity, intensity, intensity, 1.0f));
            color = u32_linear_to_srgb(color);
            *pixel++ = color;
        }
        row -= ctx->width;
    }
}

static void
render_gradient_rect_without_gamma_correction(render_context *ctx, rect2 rect) {
    i32 minx = (i32) (rect.min.x);
    i32 miny = (i32) (rect.min.y);
    i32 maxx = (i32) (rect.max.x);
    i32 maxy = (i32) (rect.max.y);

    if (minx < 0) { minx = 0; }
    if (maxx >= ctx->width) { maxx = ctx->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= ctx->height) { maxy = ctx->height; }

    u32 *row = ctx->buf + (ctx->height - 1 - miny) * ctx->width + minx;
    i32 size = maxx - minx;
    for (i32 y = miny; y < maxy; ++y) {
        u32 *pixel = row;
        for (i32 x = minx; x < maxx; ++x) {
            f32 intensity = (x - minx) * 1.0f / size;
            u32 color = rgba_to_u32(rgba(intensity, intensity, intensity, 1.0f));
            *pixel++ = color;
        }
        row -= ctx->width;
    }
}

static void
render_to_screen(render_context *ctx) {
    copy_pixels_to_texture(ctx);

    SDL_RenderCopy(ctx->renderer, ctx->texture, 0, 0);
    SDL_RenderPresent(ctx->renderer);
}

