#include "windows_aspect.h"
#include <assert.h>

static LRESULT CALLBACK procedure(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    if (fe8_windows_game_aspect_message(w, msg, wp, lp)) return msg == WM_SIZING ? TRUE : 0;
    return DefWindowProcW(w, msg, wp, lp);
}
static void check(HWND window) {
    RECT c; assert(GetClientRect(window, &c));
    assert(c.right >= 480 && c.bottom >= 320 && c.right * 2 == c.bottom * 3);
}
int main(void) {
    WNDCLASSW wc = {0}; wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(NULL); wc.lpszClassName = L"FE8AspectTest";
    assert(RegisterClassW(&wc));
    HWND window = CreateWindowW(wc.lpszClassName, L"Hidden aspect test", WS_OVERLAPPEDWINDOW,
        100,100,960,640,NULL,NULL,wc.hInstance,NULL); assert(window);
    HMENU bar = CreateMenu(), menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1, L"test"); AppendMenuW(bar, MF_POPUP, (UINT_PTR)menu, L"File");
    SetMenu(window, bar);
    SetWindowLongPtrW(window, GWLP_WNDPROC, (LONG_PTR)procedure);
    fe8_windows_fit_game_window(window); check(window);
    RECT outer, client; GetWindowRect(window, &outer); GetClientRect(window, &client);
    int bw = outer.right-outer.left-client.right, bh = outer.bottom-outer.top-client.bottom;
    for (unsigned edge = WMSZ_LEFT; edge <= WMSZ_BOTTOMRIGHT; ++edge) {
        RECT r = {100,100,1157,811};
        SendMessageW(window, WM_SIZING, edge, (LPARAM)&r);
        assert((r.right-r.left-bw)*2 == (r.bottom-r.top-bh)*3);
        if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT || edge == WMSZ_BOTTOMLEFT) assert(r.right == 1157);
        if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT) assert(r.bottom == 811);
    }
    MINMAXINFO limits = {0}; SendMessageW(window, WM_GETMINMAXINFO, 0, (LPARAM)&limits);
    assert((limits.ptMaxSize.x-bw)*2 == (limits.ptMaxSize.y-bh)*3);
    assert((limits.ptMinTrackSize.x-bw)*2 == (limits.ptMinTrackSize.y-bh)*3);
    SetWindowPos(window,NULL,0,0,1200,800,SWP_NOMOVE|SWP_NOZORDER); check(window);
    SetWindowPos(window,NULL,0,0,700,950,SWP_NOMOVE|SWP_NOZORDER); check(window);
    assert(!IsWindowVisible(window)); DestroyWindow(window);
    return 0;
}
