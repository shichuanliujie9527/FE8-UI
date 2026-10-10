#ifndef FE8_HOST_CURSOR_H
#define FE8_HOST_CURSOR_H

#include <stddef.h>
#include <stdint.h>

enum { FE8_HOST_CURSOR_WIDTH = 18, FE8_HOST_CURSOR_HEIGHT = 25 };

/* Draws the pixel-art arrow pointer whose tip is exactly at (hotspot_x,
 * hotspot_y). Pixels use the frontend's 0xAABBGGRR host-canvas format. */
void fe8_host_draw_mouse_cursor(uint32_t *pixels, size_t stride,
    int width, int height, int hotspot_x, int hotspot_y);

/* Drawable pixels per cursor pixel: half a game pixel, rounded to a whole
 * pixel so the arrow stays crisp (2 on a 1440p window, 3 on a Retina Mac). */
int fe8_host_cursor_scale(double game_pixel_scale);

#endif
