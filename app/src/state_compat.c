#include "state_compat.h"

#include <mgba/core/core.h>
#include <mgba-util/vfs.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

int fe8_load_png_state(struct mCore *core, const char *path) {
    static const uint8_t signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    struct VFile *png = VFileOpen(path, O_RDONLY);
    uint8_t header[8];
    int success = 0;
    if (!png) return 0;
    if (png->read(png, header, sizeof(header)) == sizeof(header) &&
            memcmp(header, signature, sizeof(signature)) == 0) {
        while (png->read(png, header, sizeof(header)) == sizeof(header)) {
            uint32_t length = ((uint32_t)header[0] << 24) |
                ((uint32_t)header[1] << 16) | ((uint32_t)header[2] << 8) | header[3];
            if (length > UINT32_C(16) * 1024 * 1024)
                break;
            if (memcmp(header + 4, "gbAs", 4) == 0) {
                uint8_t *compressed = malloc(length);
                size_t state_size = core->stateSize(core);
                uint8_t *raw = malloc(state_size);
                uLongf raw_size = state_size;
                if (compressed && raw && png->read(png, compressed, length) == length &&
                        uncompress(raw, &raw_size, compressed, length) == Z_OK &&
                        raw_size == state_size)
                    success = core->loadState(core, raw);
                free(raw);
                free(compressed);
                break;
            }
            if (png->seek(png, (off_t)length + 4, SEEK_CUR) < 0)
                break;
        }
    }
    png->close(png);
    return success;
}
