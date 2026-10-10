#include "host_cursor.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(void) {
    uint32_t pixels[32 * 32];
    memset(pixels, 0, sizeof(pixels));
    fe8_host_draw_mouse_cursor(pixels, 32, 32, 32, 2, 3);
    assert(pixels[3 * 32 + 2] != 0);  /* exact hotspot */
    assert(pixels[3 * 32 + 4] == 0);  /* transparent beside the tip */
    assert(pixels[11 * 32 + 6] == UINT32_C(0xFFFCF8F8)); /* white body */
    assert(pixels[11 * 32 + 2] == UINT32_C(0xFF402038)); /* outline */
    assert(pixels[26 * 32 + 13] != 0); /* lower tail */
    assert(pixels[28 * 32 + 13] == 0); /* clean lower edge */
    assert(pixels[31 * 32 + 31] == 0);

    /* Clipping at an edge must not write outside the supplied canvas. */
    memset(pixels, 0, sizeof(pixels));
    fe8_host_draw_mouse_cursor(pixels, 32, 32, 32, 31, 31);
    assert(pixels[31 * 32 + 31] != 0);

    /* Half a game pixel per cursor pixel, never blurry or vanishing. */
    assert(fe8_host_cursor_scale(4.3) == 2);  /* 1440p window */
    assert(fe8_host_cursor_scale(6.1) == 3);  /* Retina MacBook */
    assert(fe8_host_cursor_scale(1.0) == 1);
    assert(fe8_host_cursor_scale(0.0) == 1);
    assert(fe8_host_cursor_scale(40.0) == 8);
    return 0;
}
