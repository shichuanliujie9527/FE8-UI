/* Verified layouts and exact-image aliases. Add compatible translations here,
 * after validating the layout and each enabled capability. See docs/rom-profiles.md.
 * This file is included only by fe8_profile.c; it contains no game assets. */
static const Fe8NativeTradeLayout s_fe8j_native_trade = {
    .proc_pointer = 0x0203A60C,
    .script = 0x085C3FFC,
    .idle_unselected = 0x0802D76D,
    .idle_selected = 0x0802D8B9,
    .tutorial_policy = FE8_TRADE_TUTORIAL_VULNERARY,
    .tutorial_item_id = 0x6C,
    .column_x = {8, 120}, .column_width = 108,
    .first_row_y = 72, .row_height = 16,
};

static const Fe8Profile s_fe8j_map_profile = {
    .rom_title = FE8_HEADER_TITLE,
    .rom_game_code = FE8_HEADER_GAME_CODE,
    .rom_maker_code = FE8_HEADER_MAKER_CODE,
    .bm_state = 0x0202BCAC,
    .play_state = 0x0202BCEC,
    .map_size = 0x0202E4D0,
    .map_unit = 0x0202E4D4,
    .map_terrain = 0x0202E4D8,
    .map_movement = 0x0202E4DC,
    .map_range = 0x0202E4E0,
    .map_fog = 0x0202E4E4,
    .map_hidden = 0x0202E4E8,
    .map_other = 0x0202E4EC,
    .map_base_tiles = 0x085C2E54,
    .tileset_config = 0x02030B88,
    .blue_units = 0x0202BE48,
    .red_units = 0x0202CFB8,
    .green_units = 0x0202DDC8,
    .active_unit = 0x03004DF0,
    .sms_handle_array = 0x0203A014,
    .bg1_tilemap = 0x020234A8,
    .bg2_tilemap = 0x02023CA8,
    .profile_name = "Fire Emblem 8 (verified FE8J layout)",
    .rom_title_match = "FIREEMBLEM8",
    .capabilities = FE8_CAP_MAP | FE8_CAP_NATIVE_TRADE,
    .native_trade = &s_fe8j_native_trade,
};

static const Fe8RomProfileEntry s_exact_profiles[] = {
    { "BE8E", "01", {
        0x22, 0x0D, 0x1D, 0x6B, 0x5F, 0x56, 0xC9, 0xE2, 0x5E, 0xB6,
        0x66, 0xA7, 0xF3, 0xDD, 0x7C, 0x48, 0x6A, 0x75, 0x94, 0x17,
    }, &s_archanae_profile },
    /* Wolf/Fire Emblem China translation; only this exact image was tested. */
    { "BE8J", "01", {
        0x66, 0x43, 0xA9, 0x05, 0xBE, 0x94, 0x81, 0xC5, 0xE1, 0x80,
        0x7B, 0x2A, 0x01, 0x98, 0xCB, 0xB5, 0xA8, 0xD5, 0x93, 0x98,
    }, &s_fe8j_map_profile },
};
