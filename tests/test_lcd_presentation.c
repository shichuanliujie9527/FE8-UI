/* Exercise the real backend on an SDL software render target, without a
   window, GPU or game ROM. Reading the surface after present is well-defined
   for this renderer; reading a GPU back buffer after present is not. */
#include <assert.h>
#include "../app/src/host_video_sdl.c"

static uint32_t pixel(SDL_Surface *s, int x, int y) {
    return ((uint32_t *)((unsigned char *)s->pixels + y*s->pitch))[x];
}
static void near_rgb(uint32_t actual, uint32_t expected) {
    for (int c = 0; c < 3; ++c)
        assert(abs((int)((actual >> (8*c)) & 255) - (int)((expected >> (8*c)) & 255)) <= 2);
}
int main(int argc, char **argv) {
    assert(SDL_Init(0) == 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 12, 12, 32, SDL_PIXELFORMAT_RGBA32);
    assert(surface);
    Fe8HostVideo video = {0};
    Fe8HostVideoSdl *backend = calloc(1, sizeof(*backend)); assert(backend);
    video.backend = backend;
    backend->renderer = SDL_CreateSoftwareRenderer(surface); assert(backend->renderer);
    video.scaling.canvas_width = video.scaling.canvas_height = 4;
    assert(apply_layout(&video, backend));
    Fe8HostSettings settings; fe8_host_settings_init(&settings);
    uint32_t source[16]; for (int i = 0; i < 16; ++i) source[i] = 0xFFFFFFFF;
    assert(fe8_host_video_present(&video, source, NULL));
    near_rgb(pixel(surface, 0, 0), 0xFFFFFF);
    settings.lcd_enabled = 1;
    assert(fe8_host_video_present(&video, source, NULL));
    near_rgb(pixel(surface, 0, 0), 0x98E4E4);
    near_rgb(pixel(surface, 1, 0), 0xE4E498);
    near_rgb(pixel(surface, 0, 2), 0x88CCCC);
    SDL_Texture *cached = backend->lcd_mask;
    assert(fe8_host_video_present(&video, source, NULL));
    assert(cached == backend->lcd_mask);
    int w, h; SDL_RenderGetLogicalSize(backend->renderer, &w, &h);
    assert(w == 4 && h == 4);
    uint32_t dark = pixel(surface, 0, 0);
    settings.lcd_brightness = 120;
    assert(fe8_host_video_present(&video, source, NULL));
    assert(backend->lcd_brightness == 120 && (pixel(surface, 0, 0) & 255) > (dark & 255));
    uint32_t overlay_pixels[144] = {0}; overlay_pixels[0] = 0xFF123456;
    Fe8VideoOverlay overlay = {overlay_pixels, 12, 12};
    assert(fe8_host_video_present(&video, source, &overlay));
    near_rgb(pixel(surface, 0, 0), 0x123456);
    video.scaling.native_resolution = 1;
    assert(fe8_host_video_present(&video, source, NULL));
    near_rgb(pixel(surface, 0, 0), 0xFFFFFF);
    video.scaling.native_resolution = 0;
    settings.lcd_enabled = 0;
    assert(fe8_host_video_present(&video, source, NULL));
    near_rgb(pixel(surface, 0, 0), 0xFFFFFF);
    settings.lcd_enabled = 1;
    video.scaling.canvas_width = 2; video.scaling.canvas_height = 4;
    assert(apply_layout(&video, backend));
    assert(fe8_host_video_present(&video, source, NULL));
    assert(backend->lcd_width == 6 && backend->lcd_source_width == 2);
    near_rgb(pixel(surface, 0, 0), 0x0C0A08); /* letterbox untouched */
    assert((pixel(surface, 3, 0) & 255) > 128);
    int cx, cy;
    assert(fe8_host_video_event_to_canvas(&video, 1, 2, &cx, &cy) && cx == 1 && cy == 2);
    SDL_RenderGetLogicalSize(backend->renderer, &w, &h); assert(w == 2 && h == 4);
    fe8_host_video_deinit(&video); SDL_FreeSurface(surface);

    /* Optional local comparison from an original fixture or a private raw
       --capture BMP. Neither input nor output needs to enter the repository. */
    if (argc == 3) {
        SDL_Surface *input = SDL_LoadBMP(argv[1]); assert(input);
        SDL_Surface *rgba = SDL_ConvertSurfaceFormat(input, SDL_PIXELFORMAT_RGBA32, 0); assert(rgba);
        surface = SDL_CreateRGBSurfaceWithFormat(0, rgba->w*3, rgba->h*3, 32, SDL_PIXELFORMAT_RGBA32); assert(surface);
        backend = calloc(1, sizeof(*backend)); assert(backend); video.backend = backend;
        backend->renderer = SDL_CreateSoftwareRenderer(surface); assert(backend->renderer);
        video.scaling.canvas_width = rgba->w; video.scaling.canvas_height = rgba->h;
        assert(apply_layout(&video, backend)); settings.lcd_brightness = 40;
        assert(fe8_host_video_present(&video, rgba->pixels, NULL));
        assert(SDL_SaveBMP(surface, argv[2]) == 0);
        fe8_host_video_deinit(&video); SDL_FreeSurface(surface);
        SDL_FreeSurface(rgba); SDL_FreeSurface(input);
    }
    SDL_Quit(); return 0;
}
