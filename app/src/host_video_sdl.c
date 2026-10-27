#include "host_video.h"
#include "pointer_mapping.h"
#include "lcd_grid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Fe8HostVideoSdl {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *overlay;
    int overlay_width, overlay_height;
    SDL_Texture *lcd_mask;
    int lcd_width, lcd_height, lcd_source_width, lcd_source_height;
    int lcd_brightness, lcd_row_brightness;
    int lcd_gamma, lut_gamma;
    uint8_t gamma_lut[256];
    uint32_t *gamma_pixels;
    size_t gamma_capacity;
} Fe8HostVideoSdl;

static int present_lcd(Fe8HostVideo *video, Fe8HostVideoSdl *backend,
        const Fe8HostSettings *settings) {
    int output_width, output_height;
    if (SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height) != 0)
        return 0;
    int width = output_width, height = output_height;
    if ((int64_t)width * video->canvas_height > (int64_t)height * video->canvas_width)
        width = (int)((int64_t)height * video->canvas_width / video->canvas_height);
    else
        height = (int)((int64_t)width * video->canvas_height / video->canvas_width);
    if (width <= 0 || height <= 0 || (size_t)height > SIZE_MAX / 4 / (size_t)width)
        return 0;
    if (!backend->lcd_mask || backend->lcd_width != width || backend->lcd_height != height ||
            backend->lcd_source_width != video->canvas_width ||
            backend->lcd_source_height != video->canvas_height ||
            backend->lcd_brightness != settings->lcd_brightness ||
            backend->lcd_row_brightness != settings->lcd_row_brightness ||
            backend->lcd_gamma != settings->lcd_gamma) {
        uint32_t *pixels = malloc((size_t)width * height * sizeof(*pixels));
        if (!pixels) return 0;
        int made = fe8_lcd_grid(pixels, width, height, video->canvas_width, video->canvas_height,
            settings->lcd_brightness / 10.0f, settings->lcd_row_brightness / 10.0f);
        if (made && settings->lcd_gamma != 100)
            fe8_lcd_apply_gamma(pixels, pixels, (size_t)width * height, backend->gamma_lut);
        SDL_Texture *mask = made ? SDL_CreateTexture(backend->renderer,
            SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height) : NULL;
        int ok = mask && SDL_UpdateTexture(mask, NULL, pixels, width * 4) == 0 &&
            SDL_SetTextureBlendMode(mask, SDL_BLENDMODE_MOD) == 0 &&
            SDL_SetTextureScaleMode(mask, SDL_ScaleModeNearest) == 0;
        free(pixels);
        if (!ok) { SDL_DestroyTexture(mask); return 0; }
        SDL_DestroyTexture(backend->lcd_mask);
        backend->lcd_mask = mask;
        backend->lcd_width = width; backend->lcd_height = height;
        backend->lcd_source_width = video->canvas_width;
        backend->lcd_source_height = video->canvas_height;
        backend->lcd_brightness = settings->lcd_brightness;
        backend->lcd_row_brightness = settings->lcd_row_brightness;
        backend->lcd_gamma = settings->lcd_gamma;
    }
    SDL_Rect area = {(output_width - width) / 2, (output_height - height) / 2, width, height};
    /* One mask texel per physical output pixel. Use the exact same rectangle
     * for source and mask, then restore SDL's mouse-event coordinate filter. */
    int ok = SDL_RenderSetLogicalSize(backend->renderer, 0, 0) == 0 &&
        SDL_RenderSetViewport(backend->renderer, NULL) == 0 &&
        SDL_RenderSetScale(backend->renderer, 1, 1) == 0 &&
        SDL_RenderCopy(backend->renderer, backend->texture, NULL, &area) == 0 &&
        SDL_RenderCopy(backend->renderer, backend->lcd_mask, NULL, &area) == 0;
    int restored = SDL_RenderSetLogicalSize(backend->renderer, video->canvas_width, video->canvas_height);
    return ok && restored == 0;
}

static int apply_layout(Fe8HostVideo *video, Fe8HostVideoSdl *backend) {
    SDL_Texture *texture = SDL_CreateTexture(backend->renderer,
        SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
        video->scaling.canvas_width, video->scaling.canvas_height);
    if (!texture)
        return 0;
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
    SDL_DestroyTexture(backend->texture);
    backend->texture = texture;
    video->canvas_width = video->scaling.canvas_width;
    video->canvas_height = video->scaling.canvas_height;
    return SDL_RenderSetLogicalSize(backend->renderer,
        video->canvas_width, video->canvas_height) == 0;
}

int fe8_host_video_init(Fe8HostVideo *video, const char *title,
    int canvas_width, int canvas_height, int vsync_enabled) {
    Fe8HostVideoSdl *backend;
    memset(video, 0, sizeof(*video));
    video->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, canvas_width * 2, canvas_height * 2,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
    backend = calloc(1, sizeof(*backend));
    if (!video->window || !backend)
        return 0;
    video->backend = backend;
    backend->renderer = SDL_CreateRenderer(video->window, -1,
        SDL_RENDERER_ACCELERATED | (vsync_enabled ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (!backend->renderer)
        return 0;
    SDL_RenderSetIntegerScale(backend->renderer, SDL_FALSE);
    fe8_display_scaling_init(&video->scaling,
        canvas_width, canvas_height, 240, 160);
    video->base_canvas_width = canvas_width;
    video->base_canvas_height = canvas_height;
    {
        int output_width;
        int output_height;
        SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height);
        fe8_display_scaling_resize(&video->scaling, output_width, output_height);
    }
    if (!apply_layout(video, backend))
        return 0;
    fe8_host_video_set_vsync(video, vsync_enabled);
    return 1;
}

int fe8_host_video_set_vsync(Fe8HostVideo *video, int enabled) {
    Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    if (!backend || SDL_RenderSetVSync(backend->renderer, enabled ? 1 : 0) != 0)
        return 0;
    video->vsync_active = enabled != 0;
    return 1;
}

int fe8_host_video_set_shader(Fe8HostVideo *video, enum Fe8HostShader shader) {
    if (!video || shader != FE8_HOST_SHADER_OFF)
        return 0;
    video->shader = shader;
    return 1;
}

int fe8_host_video_present(Fe8HostVideo *video, const void *pixels,
        const Fe8VideoOverlay *overlay) {
    Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    if (!backend)
        return 0;
    const Fe8HostSettings *settings = fe8_host_settings_current();
    int lcd_active = settings && settings->lcd_enabled && !video->scaling.native_resolution;
    if (lcd_active && settings->lcd_gamma != 100) {
        /* pow(source * mask, 1/gamma) factorises into two terms. Transform
         * source with a cached LUT, and transform the cached mask once. This
         * lifts midtones without an exposure multiplier's highlight clipping. */
        if (backend->lut_gamma != settings->lcd_gamma || !backend->gamma_pixels) {
            fe8_lcd_gamma_lut(backend->gamma_lut, settings->lcd_gamma);
            backend->lut_gamma = settings->lcd_gamma;
        }
        size_t count = (size_t)video->canvas_width * video->canvas_height;
        if (count > SIZE_MAX / sizeof(*backend->gamma_pixels)) return 0;
        if (count > backend->gamma_capacity) {
            uint32_t *buffer = realloc(backend->gamma_pixels, count * sizeof(*buffer));
            if (!buffer) return 0;
            backend->gamma_pixels = buffer; backend->gamma_capacity = count;
        }
        fe8_lcd_apply_gamma(backend->gamma_pixels, pixels, count, backend->gamma_lut);
        pixels = backend->gamma_pixels;
    }
    if (SDL_UpdateTexture(backend->texture, NULL, pixels,
            video->canvas_width * 4) != 0)
        return 0;
    SDL_SetRenderDrawColor(backend->renderer, 8, 10, 12, 255);
    SDL_RenderClear(backend->renderer);
    if (lcd_active) {
        if (!present_lcd(video, backend, settings)) return 0;
    } else if (SDL_RenderCopy(backend->renderer, backend->texture, NULL, NULL) != 0)
        return 0;
    if (overlay && overlay->pixels) {
        if (!backend->overlay || backend->overlay_width != overlay->width ||
                backend->overlay_height != overlay->height) {
            SDL_Texture *texture = SDL_CreateTexture(backend->renderer,
                SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, overlay->width, overlay->height);
            if (!texture) return 0;
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
            SDL_DestroyTexture(backend->overlay);
            backend->overlay = texture;
            backend->overlay_width = overlay->width; backend->overlay_height = overlay->height;
        }
        if (SDL_UpdateTexture(backend->overlay, NULL, overlay->pixels, overlay->width * 4) != 0)
            return 0;
        /* Draw in physical pixels, then restore the mouse-event transform. */
        SDL_RenderSetLogicalSize(backend->renderer, 0, 0);
        SDL_RenderSetViewport(backend->renderer, NULL);
        SDL_RenderSetScale(backend->renderer, 1, 1);
        int drawn = SDL_RenderCopy(backend->renderer, backend->overlay, NULL, NULL);
        int restored = SDL_RenderSetLogicalSize(backend->renderer, video->canvas_width, video->canvas_height);
        if (drawn != 0 || restored != 0) return 0;
    }
    SDL_RenderPresent(backend->renderer);
    return 1;
}

int fe8_host_video_window_to_canvas(const Fe8HostVideo *video,
    int window_x, int window_y, int *canvas_x, int *canvas_y) {
    const Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    int window_width;
    int window_height;
    int output_width;
    int output_height;
    if (!backend || !backend->renderer)
        return 0;
    SDL_GetWindowSize(video->window, &window_width, &window_height);
    SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height);
    return fe8_pointer_window_to_canvas(window_width, window_height,
        output_width, output_height, video->canvas_width, video->canvas_height,
        window_x, window_y, canvas_x, canvas_y);
}

int fe8_host_video_event_to_canvas(const Fe8HostVideo *video,
    int event_x, int event_y, int *canvas_x, int *canvas_y) {
    /* The SDL renderer's event filter has already mapped these coordinates
       to its logical canvas. Applying window scaling again shifts clicks
       away from the hovered label after a resize or density change. */
    if (!video || !video->backend || !canvas_x || !canvas_y ||
            event_x < 0 || event_y < 0 ||
            event_x >= video->canvas_width || event_y >= video->canvas_height)
        return 0;
    *canvas_x = event_x;
    *canvas_y = event_y;
    return 1;
}

int fe8_host_video_adjust_zoom(
    Fe8HostVideo *video, double wheel_delta, double sensitivity) {
    Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    if (!backend || !backend->renderer)
        return 0;
    if (!fe8_display_scaling_adjust(
            &video->scaling, wheel_delta, sensitivity))
        return 0;
    return apply_layout(video, backend);
}

int fe8_host_video_refresh_layout(Fe8HostVideo *video) {
    Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    int output_width;
    int output_height;
    if (!backend || !backend->renderer)
        return 0;
    SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height);
    if (!fe8_display_scaling_resize(
            &video->scaling, output_width, output_height))
        return 0;
    return apply_layout(video, backend);
}

int fe8_host_video_set_content_density(Fe8HostVideo *video, int density) {
    Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    int output_width;
    int output_height;
    if (!backend || density < 0 || density > 3)
        return 0;
    video->scaling.native_resolution = density == 0;
    if (density == 0) density = 1;
    video->scaling.minimum_canvas_width = video->base_canvas_width * density;
    video->scaling.minimum_canvas_height = video->base_canvas_height * density;
    SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height);
    fe8_display_scaling_resize(&video->scaling, output_width, output_height);
    return apply_layout(video, backend);
}

void fe8_host_video_log_status(const Fe8HostVideo *video) {
    const Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    int window_width;
    int window_height;
    int output_width;
    int output_height;
    if (!backend)
        return;
    SDL_GetWindowSize(video->window, &window_width, &window_height);
    SDL_GetRendererOutputSize(backend->renderer, &output_width, &output_height);
    fprintf(stderr, "Display: window=%dx%d output=%dx%d logical=%dx%d scale=%.2fx backend=SDL\n",
        window_width, window_height, output_width, output_height,
        video->canvas_width, video->canvas_height, video->scaling.pixel_scale);
    const Fe8HostSettings *settings = fe8_host_settings_current();
    if (settings && settings->lcd_enabled)
        fprintf(stderr, "LCD: mask=%.1f rows=%.1f gamma=%.2f\n",
            settings->lcd_brightness / 10.0, settings->lcd_row_brightness / 10.0,
            settings->lcd_gamma / 100.0);
}

void fe8_host_video_deinit(Fe8HostVideo *video) {
    Fe8HostVideoSdl *backend = video ? video->backend : NULL;
    if (!video)
        return;
    if (backend) {
        SDL_DestroyTexture(backend->texture);
        SDL_DestroyTexture(backend->overlay);
        SDL_DestroyTexture(backend->lcd_mask);
        free(backend->gamma_pixels);
        SDL_DestroyRenderer(backend->renderer);
        free(backend);
    }
    SDL_DestroyWindow(video->window);
    memset(video, 0, sizeof(*video));
}
