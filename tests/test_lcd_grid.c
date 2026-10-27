#include "lcd_grid.h"
#include <assert.h>
#include <math.h>
#include <string.h>

int main(void) {
    uint32_t pixels[36], brighter[36], fallback[36];
    /* Independent golden values from lcd3x's default fragment expression,
       sampled at 3x pixel centres. Each triplet is R,G,B, not a grey scanline. */
    const unsigned char expected[3][3][3] = {
        {{228,228,152},{152,228,228},{228,152,228}},
        {{216,216,144},{144,216,216},{216,144,216}},
        {{204,204,136},{136,204,204},{204,136,204}}};
    assert(fe8_lcd_grid(pixels, 6, 6, 2, 2, 4, 16));
    for (int y = 0; y < 6; ++y) for (int x = 0; x < 6; ++x) {
        assert(pixels[y*6+x] >> 24 == 255);
        for (int c = 0; c < 3; ++c)
            assert(((pixels[y*6+x] >> (8*c)) & 255) == expected[y%3][x%3][c]);
    }
    assert(fe8_lcd_grid(brighter, 6, 6, 2, 2, 12, 32));
    for (int i = 0; i < 36; ++i) for (int c = 0; c < 3; ++c)
        assert(((brighter[i] >> (8*c)) & 255) >= ((pixels[i] >> (8*c)) & 255));
    assert(fe8_lcd_grid(fallback, 6, 6, 2, 2, NAN, -1));
    assert(!memcmp(fallback, pixels, sizeof pixels));
    assert(!fe8_lcd_grid(NULL, 6, 6, 2, 2, 4, 16));
    assert(!fe8_lcd_grid(pixels, 0, 6, 2, 2, 4, 16));
    assert(!fe8_lcd_grid(pixels, 6, 6, 0, 2, 4, 16));
    /* Noninteger magnification must follow source pixels, not a fixed 3px tile. */
    assert(fe8_lcd_grid(pixels, 5, 7, 2, 3, 4, 16));
    assert(pixels[0] != pixels[3] && pixels[0] != pixels[15]);
    return 0;
}
