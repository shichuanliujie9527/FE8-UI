#ifndef FE8_HOST_VIDEO_H
#define FE8_HOST_VIDEO_H

#include <SDL.h>

#include "host_settings.h"
#include "display_scaling.h"

/* Optional straight-alpha RGBA plane in drawable pixels, after game shaders. */
typedef struct Fe8VideoOverlay {
    const void *pixels;
    int width, height;
} Fe8VideoOverlay;

typedef struct Fe8HostVideo {
    SDL_Window *window;
    void *backend;
    int canvas_width;
    int canvas_height;
    int base_canvas_width;
    int base_canvas_height;
    Fe8DisplayScaling scaling;
    int vsync_active;
    enum Fe8HostShader shader;
    SDL_Rect source_rect; /* Zero size means the full canvas. SDL presentation crop. */
} Fe8HostVideo;

static inline SDL_Rect fe8_host_video_source(const Fe8HostVideo *video) {
    if (video->source_rect.w > 0 && video->source_rect.h > 0) return video->source_rect;
    return (SDL_Rect){0, 0, video->canvas_width, video->canvas_height};
}

/* Select only the authoritative 240x160 frame when no extended scene is visible.
 * This changes presentation and input transforms, not the emulation/map canvas. */
int fe8_host_video_set_native_frame(Fe8HostVideo *video, int enabled, int x, int y);

int fe8_host_video_init(Fe8HostVideo *video, const char *title,
    int canvas_width, int canvas_height, int vsync_enabled);
int fe8_host_video_set_vsync(Fe8HostVideo *video, int enabled);
int fe8_host_video_set_shader(Fe8HostVideo *video, enum Fe8HostShader shader);
int fe8_host_video_present(Fe8HostVideo *video, const void *pixels,
    const Fe8VideoOverlay *overlay);
int fe8_host_video_window_to_canvas(const Fe8HostVideo *video,
    int window_x, int window_y, int *canvas_x, int *canvas_y);
/* SDL_RenderSetLogicalSize transforms mouse-event coordinates, but not
   SDL_GetMouseState. Keep event and raw window positions distinct. */
int fe8_host_video_event_to_canvas(const Fe8HostVideo *video,
    int event_x, int event_y, int *canvas_x, int *canvas_y);
int fe8_host_video_adjust_zoom(
    Fe8HostVideo *video, double wheel_delta, double sensitivity);
int fe8_host_video_refresh_layout(Fe8HostVideo *video);
/* Density 0 selects native drawable resolution without changing game zoom. */
int fe8_host_video_set_content_density(Fe8HostVideo *video, int density);
void fe8_host_video_log_status(const Fe8HostVideo *video);
void fe8_host_video_deinit(Fe8HostVideo *video);

#endif
