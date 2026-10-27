#include "windows_aspect.h"

static int smaller(int a, int b) { return a < b ? a : b; }
static void chrome(HWND window, int *width, int *height) {
    RECT outer, client;
    GetWindowRect(window, &outer); GetClientRect(window, &client);
    *width = outer.right - outer.left - (client.right - client.left);
    *height = outer.bottom - outer.top - (client.bottom - client.top);
}
static int work_units(HWND window, int border_w, int border_h, MONITORINFO *monitor) {
    monitor->cbSize = sizeof(*monitor);
    if (!GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), monitor)) return 160;
    return smaller((monitor->rcWork.right - monitor->rcWork.left - border_w) / 3,
        (monitor->rcWork.bottom - monitor->rcWork.top - border_h) / 2);
}
int fe8_windows_game_aspect_message(HWND window, UINT message, WPARAM wp, LPARAM lp) {
    if (message != WM_SIZING && message != WM_GETMINMAXINFO && message != WM_WINDOWPOSCHANGING)
        return 0;
    int bw, bh; chrome(window, &bw, &bh);
    if (message == WM_GETMINMAXINFO) {
        MONITORINFO monitor = {0};
        int units = work_units(window, bw, bh, &monitor);
        MINMAXINFO *limits = (MINMAXINFO *)lp;
        limits->ptMinTrackSize = (POINT){480 + bw, 320 + bh};
        limits->ptMaxSize = (POINT){3 * units + bw, 2 * units + bh};
        limits->ptMaxTrackSize = limits->ptMaxSize;
        limits->ptMaxPosition = (POINT){
            monitor.rcWork.left - monitor.rcMonitor.left +
                (monitor.rcWork.right - monitor.rcWork.left - limits->ptMaxSize.x) / 2,
            monitor.rcWork.top - monitor.rcMonitor.top +
                (monitor.rcWork.bottom - monitor.rcWork.top - limits->ptMaxSize.y) / 2};
        return 1;
    }
    if (message == WM_SIZING) {
        RECT *r = (RECT *)lp;
        int units = (wp == WMSZ_TOP || wp == WMSZ_BOTTOM) ?
            (r->bottom - r->top - bh + 1) / 2 : (r->right - r->left - bw + 1) / 3;
        if (units < 160) units = 160;
        int width = units * 3 + bw, height = units * 2 + bh;
        if (wp == WMSZ_LEFT || wp == WMSZ_TOPLEFT || wp == WMSZ_BOTTOMLEFT) r->left = r->right - width;
        else r->right = r->left + width;
        if (wp == WMSZ_TOP || wp == WMSZ_TOPLEFT || wp == WMSZ_TOPRIGHT) r->top = r->bottom - height;
        else r->bottom = r->top + height;
        return 1;
    }
    WINDOWPOS *position = (WINDOWPOS *)lp;
    /* Also constrain snap/programmatic sizing. The normal subclass chain still
       receives this message, so SDL sees the resulting size/DPI changes. */
    if (!(position->flags & SWP_NOSIZE) && !IsIconic(window) && position->cx > bw && position->cy > bh) {
        int units = smaller((position->cx - bw) / 3, (position->cy - bh) / 2);
        if (units < 160) units = 160;
        position->cx = units * 3 + bw; position->cy = units * 2 + bh;
    }
    return 0;
}
void fe8_windows_fit_game_window(HWND window) {
    int bw, bh; chrome(window, &bw, &bh);
    RECT client; GetClientRect(window, &client);
    MONITORINFO monitor = {0};
    int units = smaller(client.right / 3, work_units(window, bw, bh, &monitor));
    if (units < 160) units = 160;
    SetWindowPos(window, NULL, 0, 0, units * 3 + bw, units * 2 + bh,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}
