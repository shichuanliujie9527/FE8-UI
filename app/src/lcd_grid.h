#ifndef FE8_LCD_GRID_H
#define FE8_LCD_GRID_H

#include <stdint.h>

/* RGBA32 modulation mask at presentation resolution. Source dimensions are
 * the full game canvas, including extended terrain, before desktop scaling. */
int fe8_lcd_grid(uint32_t *pixels, int width, int height,
    int source_width, int source_height, float lcd_brightness, float row_brightness);

#endif
