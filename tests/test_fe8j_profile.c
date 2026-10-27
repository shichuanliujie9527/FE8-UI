#include "address_space.h"
#include "fe8_catalog.h"
#include "prebattle_inventory.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t ram[0x40000];
static uint8_t iwram[0x8000];
static unsigned writes;

static void put32(uint8_t *data, uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) data[i] = (uint8_t)(value >> (i * 8));
}
static void forbidden_write(void *context, uint32_t address, uint8_t value) {
    (void)context; (void)address; (void)value;
    ++writes;
}

int main(void) {
    static const uint8_t digest[20] = {
        0x66, 0x43, 0xA9, 0x05, 0xBE, 0x94, 0x81, 0xC5, 0xE1, 0x80,
        0x7B, 0x2A, 0x01, 0x98, 0xCB, 0xB5, 0xA8, 0xD5, 0x93, 0x98,
    };
    uint8_t *rom = calloc(0x600000, 1);
    Fe8AddressSpace space;
    Fe8Snapshot snapshot;
    Fe8Catalog catalog;
    assert(rom);
    memcpy(rom + 0xAC, "BE8J01", 6);
    fe8_address_space_init(&space, NULL, NULL);
    assert(fe8_address_space_add(&space, 0x08000000, rom, 0x600000));
    assert(fe8_address_space_add(&space, 0x02000000, ram, sizeof(ram)));
    assert(fe8_address_space_add(&space, 0x03000000, iwram, sizeof(iwram)));
    Fe8MemoryReader memory = {&space, fe8_address_space_read8};
    assert(!fe8_detect_supported_rom(NULL));
    assert(!fe8_detect_supported_rom(&memory)); /* Header alone is insufficient. */
    assert(!fe8_profile_for_rom(&memory)->native_trade);
    memcpy(space.rom_sha1, digest, sizeof(digest));
    assert(fe8_detect_supported_rom(&memory));
    assert(!fe8_detect_fe8u_family(&memory));
    const Fe8Profile *profile = fe8_profile_for_rom(&memory);
    assert(!fe8_profile_has(profile, FE8_CAP_INVENTORY) && profile->inventory.item_table == 0);
    assert(profile->native_trade->proc_pointer == 0x0203A60C);

    /* Synthetic JP memory layout: 8x6 map, one unit, no active cursor motion. */
    put32(ram + 0x2E4D0, 8 | (6u << 16));
    put32(ram + 0x2E4D8, 0x02031000);
    put32(rom + 0x5C2E54, 0x02032000);
    for (unsigned y = 0; y < 6; ++y) {
        put32(ram + 0x31000 + 4 * y, 0x02031100 + 8 * y);
        memset(ram + 0x31100 + 8 * y, 1, 8);
        put32(ram + 0x32000 + 4 * y, 0x02032100 + 16 * y);
    }
    put32(ram + 0x2BE48, 0x08010000);
    assert(fe8_extract_snapshot(&memory, profile, &snapshot));
    assert(snapshot.map_width == 8 && snapshot.map_height == 6);
    assert(snapshot.terrain[47] == 1 && snapshot.visible_unit_count == 1);
    assert(snapshot.base_tile_rows == 0x02032000);
    Fe8Profile disabled_map = *profile;
    disabled_map.capabilities &= ~FE8_CAP_MAP;
    Fe8LiveState live;
    assert(!fe8_extract_live_state(&memory, &disabled_map, &live));
    put32(ram + 0x31000, 0x02040000); /* Out-of-range row must fail closed. */
    assert(!fe8_extract_snapshot(&memory, profile, &snapshot));
    put32(ram + 0x31000, 0x02031100);
    assert(fe8_extract_snapshot(&memory, profile, &snapshot));

    Fe8Profile writable_fixture = *profile;
    writable_fixture.capabilities |= FE8_CAP_INVENTORY;
    assert(fe8_inventory_management_available(&memory, &writable_fixture));
    assert(!fe8_inventory_management_available(&memory, profile));
    assert(!fe8_catalog_init(&memory, profile, &catalog) && !catalog.valid);
    Fe8MemoryWriter writer = {NULL, forbidden_write};
    Fe8InventoryEndpoint a = {FE8_INVENTORY_ENDPOINT_UNIT, profile->blue_units, 0};
    Fe8InventoryEndpoint b = {FE8_INVENTORY_ENDPOINT_UNIT, profile->blue_units, 1};
    assert(!fe8_swap_inventory_endpoints(&memory, &writer, profile, a, 0, b, 0));
    assert(!fe8_swap_prebattle_items(&memory, &writer, profile,
        profile->blue_units, 0, 0, profile->blue_units, 1, 0));
    assert(writes == 0);

    space.rom_sha1[0] ^= 1;
    assert(!fe8_detect_supported_rom(&memory));
    space.rom_sha1[0] ^= 1;
    rom[0xAF] = 'X';
    assert(!fe8_detect_supported_rom(&memory));
    memcpy(rom + 0xAC, "BE8E01", 6);
    assert(fe8_detect_supported_rom(&memory) && fe8_detect_fe8u_family(&memory));
    assert(fe8_profile_has(fe8_profile_for_rom(&memory), FE8_CAP_INVENTORY));
    free(rom);
    puts("FE8J: exact identity, validated map and inventory write rejection passed");
    return 0;
}
