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
    u32 result = (u32) ((r << 24) | (g << 16) | (b << 8) | (a << 0));
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

