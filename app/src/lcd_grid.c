#include "lcd_grid.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

/* Gigaherz's public-domain lcd3x (libretro/slang-shaders): the original
 * two-axis sinusoidal mask, evaluated once per layout/parameter change.
 * SDL multiplies it with the nearest-neighbour game image on the GPU.
 * See docs/lcd3x.md for the reference preset and precision differences. */
int fe8_lcd_grid(uint32_t *pixels, int width, int height,
        int source_width, int source_height, float lcd_brightness, float row_brightness) {
    const double pi = 3.14159265358979323846;
    if (!pixels || width <= 0 || height <= 0 || source_width <= 0 || source_height <= 0 ||
            (size_t)width > SIZE_MAX / (3 * sizeof(double)) ||
            (size_t)height > SIZE_MAX / sizeof(*pixels) / (size_t)width)
        return 0;
    if (!(lcd_brightness >= 1 && lcd_brightness <= 12)) lcd_brightness = 4;
    if (!(row_brightness >= 1 && row_brightness <= 32)) row_brightness = 16;
    double *columns = malloc((size_t)width * 3 * sizeof(*columns));
    if (!columns) return 0;
    for (int x = 0; x < width; ++x) {
        double angle = 2 * pi * ((x + .5) * source_width / width);
        for (int channel = 0; channel < 3; ++channel)
            columns[x * 3 + channel] = (lcd_brightness +
                sin(angle + pi / 2 - channel * 2 * pi / 3)) / (lcd_brightness + 1);
    }
    for (int y = 0; y < height; ++y) {
        double row = (row_brightness + sin(2 * pi * ((y + .5) * source_height / height))) /
            (row_brightness + 1);
        for (int x = 0; x < width; ++x) {
            uint32_t pixel = UINT32_C(0xFF000000);
            for (int channel = 0; channel < 3; ++channel) {
                unsigned value = (unsigned)(255 * row * columns[x * 3 + channel] + .5);
                if (value > 255) value = 255;
                pixel |= value << (channel * 8);
            }
            pixels[(size_t)y * width + x] = pixel;
        }
    }
    free(columns);
    return 1;
}
