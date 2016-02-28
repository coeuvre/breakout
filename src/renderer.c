static void
copy_pixels_to_texture(render_context *ctx) {
    SDL_UpdateTexture(ctx->texture, 0, ctx->buf, ctx->width * 4);
}

static inline u32
rgba_to_uint32(vec4 color) {
    u8 r = (u8) (color.r * 255.0f);
    u8 g = (u8) (color.g * 255.0f);
    u8 b = (u8) (color.b * 255.0f);
    u8 a = (u8) (color.a * 255.0f);

    // 0xRRGGBBAA
    u32 result = (u32) ((r << 24) | (g << 16) | (b << 8) | (a << 0));
    return result;
}

static void
render_rect(render_context *ctx, rect2 rect, vec4 rgba) {
    i32 minx = (i32) (rect.min.x);
    i32 miny = (i32) (rect.min.y);
    i32 maxx = (i32) (rect.max.x);
    i32 maxy = (i32) (rect.max.y);

    if (minx < 0) { minx = 0; }
    if (maxx >= ctx->width) { maxx = ctx->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= ctx->height) { maxy = ctx->height; }

    u32 color = rgba_to_uint32(rgba);
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
            u32 color = rgba_to_uint32(rgba(intensity, intensity, intensity, 1.0f));
            *pixel++ = color;
        }
        row -= ctx->width;
    }
}

static void
render_gradient_rect_with_gamma_correction(render_context *ctx, rect2 rect) {
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
            intensity = powf(intensity, 1.0f / 2.2f);
            u32 color = rgba_to_uint32(rgba(intensity, intensity, intensity, 1.0f));
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

