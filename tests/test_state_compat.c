#include "state_compat.h"
#include <mgba/core/core.h>
#include <mgba-util/vfs.h>
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

static const unsigned char payload[] = "synthetic emulator state, no game data";
static int loads;

static size_t state_size(struct mCore *core) {
    (void)core;
    return sizeof(payload);
}

static bool load_state(struct mCore *core, const void *data) {
    (void)core;
    assert(!memcmp(data, payload, sizeof(payload)));
    ++loads;
    return true;
}

static void chunk(struct VFile *file, const char *type,
        const unsigned char *data, uint32_t length) {
    unsigned char header[8] = {
        (unsigned char)(length >> 24), (unsigned char)(length >> 16),
        (unsigned char)(length >> 8), (unsigned char)length
    };
    memcpy(header + 4, type, 4);
    assert(file->write(file, header, sizeof(header)) == sizeof(header));
    if (length) assert(file->write(file, data, length) == length);
    uLong crc = crc32(0, (const Bytef *)type, 4);
    if (length) crc = crc32(crc, data, length);
    unsigned char checksum[4] = {
        (unsigned char)(crc >> 24), (unsigned char)(crc >> 16),
        (unsigned char)(crc >> 8), (unsigned char)crc
    };
    assert(file->write(file, checksum, sizeof(checksum)) == sizeof(checksum));
}

int main(void) {
    /* Escaped UTF-8 also works with MSVC's non-UTF-8 source code page. */
    const char *path = "state-compat-\xE4\xB8\xAD\xE6\x96\x87 fixture.tmp";
    struct mCore core = {0};
    core.stateSize = state_size;
    core.loadState = load_state;
    unsigned char compressed[128];
    uLongf length = sizeof(compressed);
    assert(compress(compressed, &length, payload, sizeof(payload)) == Z_OK);
    static const unsigned char signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    static const unsigned char ihdr[13] = {0, 0, 0, 1, 0, 0, 0, 1, 8, 6, 0, 0, 0};

    for (int variant = 0; variant < 4; ++variant) {
        struct VFile *file = VFileOpen(path, O_WRONLY | O_CREAT | O_TRUNC);
        assert(file);
        if (variant == 0 || variant == 1) {
            assert(file->write(file, signature, sizeof(signature)) == sizeof(signature));
            chunk(file, "IHDR", ihdr, sizeof(ihdr));
            if (variant == 1) --length; /* Truncated compressed stream. */
            chunk(file, "gbAs", compressed, (uint32_t)length);
            chunk(file, "IEND", NULL, 0);
        } else if (variant == 2) {
            assert(file->write(file, "not a PNG", 9) == 9);
        } else {
            assert(file->write(file, signature, sizeof(signature)) == sizeof(signature));
            chunk(file, "IEND", NULL, 0); /* No state chunk. */
        }
        assert(file->close(file));
        assert(fe8_load_png_state(&core, path) == (variant == 0));
        assert(loads == 1);
    }
#ifdef _WIN32
    wchar_t wide[128];
    assert(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 128));
    assert(DeleteFileW(wide));
#else
    assert(remove(path) == 0);
#endif
    assert(!fe8_load_png_state(&core, path));
    assert(loads == 1);
    puts("Legacy PNG state: Unicode path and invalid inputs passed");
    return 0;
}
