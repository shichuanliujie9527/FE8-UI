#ifndef FE8_NATIVE_TRADE_H
#define FE8_NATIVE_TRADE_H
#include "fe8_profile.h"

typedef struct Fe8NativeTrade {
    const Fe8NativeTradeLayout *layout;
    uint32_t proc, units[2];
    uint16_t items[2][5];
    uint8_t selectable[2][5];
    uint8_t column, row, phase, tutorial, blocked, selected_column, selected_row;
} Fe8NativeTrade;
typedef struct Fe8TradeMouse {
    int active, column, row, frames, press, release;
    uint32_t key;
    Fe8NativeTrade initial;
} Fe8TradeMouse;

int fe8_native_trade_read(const Fe8MemoryReader *memory, const Fe8Profile *profile, Fe8NativeTrade *out);
/* Native 240x160 coordinates. Returns 1 only for a selectable item/extra cell. */
int fe8_trade_mouse_click(Fe8TradeMouse *mouse, const Fe8NativeTrade *state, int x, int y);
void fe8_trade_mouse_cancel(Fe8TradeMouse *mouse);
uint32_t fe8_trade_mouse_update(Fe8TradeMouse *mouse, const Fe8NativeTrade *state, int valid);
#endif
