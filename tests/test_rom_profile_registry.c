#include "fe8_profile.h"

#include <assert.h>
#include <string.h>

int main(void) {
    /* Artificial identities: no untested translation is registered in the app. */
    Fe8Profile shared = {0}, alternate = {0};
    shared.capabilities = FE8_CAP_MAP;
    alternate.capabilities = FE8_CAP_MAP | FE8_CAP_NATIVE_TRADE;
    Fe8RomProfileEntry aliases[] = {
        {"BE8J", "01", {1}, &shared},
        {"BE8P", "01", {2}, &shared},
        {"BE8P", "01", {3}, &alternate},
    };
    uint8_t digest[20] = {1};
    assert(fe8_profile_match_exact("BE8J", "01", digest, aliases, 3) == &shared);
    digest[0] = 2;
    assert(fe8_profile_match_exact("BE8P", "01", digest, aliases, 3) == &shared);
    assert(!fe8_profile_match_exact("BE8J", "01", digest, aliases, 3));
    assert(!fe8_profile_match_exact("BE8P", "02", digest, aliases, 3));
    digest[19] = 1;
    assert(!fe8_profile_match_exact("BE8P", "01", digest, aliases, 3));
    digest[19] = 0; digest[0] = 3;
    const Fe8Profile *matched = fe8_profile_match_exact("BE8P", "01", digest, aliases, 3);
    assert(matched == &alternate);
    assert(fe8_profile_has(matched, FE8_CAP_NATIVE_TRADE));
    assert(!fe8_profile_has(matched, FE8_CAP_INVENTORY));
    assert(!fe8_profile_has(NULL, FE8_CAP_MAP));
    assert(!fe8_profile_match_exact("BE8P", "01", NULL, aliases, 3));
    assert(!fe8_profile_match_exact("BE8P", "01", digest, NULL, 3));
    assert(!fe8_profile_match_exact("BE8P", "01", digest, aliases, 0));
    aliases[2].profile = NULL;
    assert(!fe8_profile_match_exact("BE8P", "01", digest, aliases, 3));
    return 0;
}
