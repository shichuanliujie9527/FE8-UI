#ifndef FE8_WINDOWS_ASPECT_H
#define FE8_WINDOWS_ASPECT_H
#include <windows.h>
int fe8_windows_game_aspect_message(HWND window, UINT message, WPARAM wp, LPARAM lp);
void fe8_windows_fit_game_window(HWND window);
#endif
