#ifndef FE8_STATE_COMPAT_H
#define FE8_STATE_COMPAT_H

struct mCore;

/* Read a legacy PNG gbAs payload through mGBA's UTF-8-aware file API. */
int fe8_load_png_state(struct mCore *core, const char *path);

#endif
