#include "host_cursor.h"

static uint32_t cursor_color(char code) {
    /* Host canvas colors are 0xAABBGGRR. */
    switch (code) {
    case 'K': return UINT32_C(0xFF402038); /* plum ink outline */
    case 'W': return UINT32_C(0xFFFCF8F8); /* paper white */
    default: return 0;
    }
}

int fe8_host_cursor_scale(double game_pixel_scale) {
    int scale = (int)(game_pixel_scale / 2.0 + 0.5);
    if (scale < 1)
        return 1;
    return scale > 8 ? 8 : scale;
}

void fe8_host_draw_mouse_cursor(uint32_t *pixels, size_t stride,
    int width, int height, int hotspot_x, int hotspot_y) {
    /* Mouse Cursor Sprite by Squidjees on Fire Emblem Universe. Its white
     * body and one-pixel plum outline stay readable over FE8 terrain and menus. */
    static const char *const art[FE8_HOST_CURSOR_HEIGHT] = {
        "KK................",
        "KWK...............",
        "KWWK..............",
        "KWWWK.............",
        "KWWWWK............",
        "KWWWWWK...........",
        "KWWWWWWK..........",
        "KWWWWWWWK.........",
        "KWWWWWWWWK........",
        "KWWWWWWWWWK.......",
        "KWWWWWWWWWWK......",
        "KWWWWWWWWWWWK.....",
        "KWWWWWWWWWWWWK....",
        "KWWWWWWWWWWWWWK...",
        "KWWWWWWWWWWWWWWK..",
        "KWWWWWWWWWWWWWWWK.",
        "KWWWWWWWWWWWWWWWWK",
        "KWWWWWWWWWKKKKKKKK",
        "KWWWWWKWWWK.......",
        "KWWWWKKWWWWK......",
        "KWWWK..KWWWK......",
        "KWWK...KWWWWK.....",
        "KWK.....KWWWK.....",
        "KK......KWWWK.....",
        ".........KKKK.....",
    };
    int y;
    if (!pixels || stride < (size_t)width || width <= 0 || height <= 0)
        return;
    for (y = 0; y < FE8_HOST_CURSOR_HEIGHT; ++y) {
        int destination_y = hotspot_y + y;
        int x;
        if (destination_y < 0 || destination_y >= height)
            continue;
        for (x = 0; x < FE8_HOST_CURSOR_WIDTH; ++x) {
            int destination_x = hotspot_x + x;
            uint32_t color = cursor_color(art[y][x]);
            if (!color || destination_x < 0 || destination_x >= width)
                continue;
            pixels[(size_t)destination_y * stride + destination_x] = color;
        }
    }
}
