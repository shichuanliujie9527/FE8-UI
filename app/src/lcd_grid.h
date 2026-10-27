#ifndef FE8_LCD_GRID_H
#define FE8_LCD_GRID_H

#include <stdint.h>
#include <stddef.h>

/* RGBA32 modulation mask at presentation resolution. Source dimensions are
 * the full game canvas, including extended terrain, before desktop scaling. */
int fe8_lcd_grid(uint32_t *pixels, int width, int height,
    int source_width, int source_height, float lcd_brightness, float row_brightness);

/* Optional FE8-UI brightness compensation, separate from the lcd3x mask.
 * 100 is identity; 100..220 represents gamma 1.00..2.20. Alpha is unchanged. */
void fe8_lcd_gamma_lut(uint8_t lut[256], int gamma_percent);
void fe8_lcd_apply_gamma(uint32_t *out, const uint32_t *in, size_t count,
    const uint8_t lut[256]);

#endif
