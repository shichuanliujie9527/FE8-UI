# Extending ROM support

## Emulation and frontend extensions

libmGBA runs the game and owns CPU execution, graphics, sound, saves and input.
A ROM can boot correctly without supporting FE8-UI's extended map, mouse map
navigation, relocated HUD or Armory. These features interpret game-specific
memory structures. They require a compatible, validated layout.

The upstream frontend already supports the FE8U header family and several
FE8-derived hacks. This change preserves those paths and adds exact-image
registration for verified layouts from other regions. An unknown non-US image
continues with ordinary emulation and native A/B input; it does not inherit
FE8U addresses merely because its filename or language looks familiar.

## Identity, layout and capabilities

`app/src/fe8_profile_registry.inc` contains exact image identities and shared
layouts. Each registration matches the complete SHA-1 plus the game and maker
codes. Two verified translations may point to the same layout. A relocated
translation can instead supply another layout without changing the matcher.
SHA-1 is an identity key here, not a signature or a source-authentication claim.

Each `Fe8Profile` explicitly enables independent capabilities:

| Capability | What it permits |
| --- | --- |
| `FE8_CAP_MAP` | Validated FE8 map structures and extended map input/rendering |
| `FE8_CAP_INVENTORY` | Armory catalog access and validated inventory transactions |
| `FE8_CAP_NATIVE_HUD` | Relocating supported native HUD panels |
| `FE8_CAP_HP_BAR_PATCH` | Probing the known FE8U HP-bar patch hook |
| `FE8_CAP_NATIVE_TRADE` | Read-only native trade state and ordinary direction/A input |

Map support does not grant inventory write access. Native trade does not grant
Armory access: the game performs the transfer using its own input handlers.
All existing pointer, dimension and visual validation remains in place.

The trade layout stores process addresses, callback identities, panel geometry,
tutorial policy and tutorial item ID. The adapter implements the FE8 two-column,
five-slot process ABI. It contains no Chinese-language detection. A new game
engine or a changed process ABI may need a new adapter; registering a hash alone
cannot make FE6, FE7 or arbitrary hacks use FE8's structures safely.

## Adding a translation or regional version

1. Identify the exact image using `tools/inspect-rom.ps1`; keep the ROM and all
   saves local. A language name, filename or header alone is insufficient.
2. Verify map addresses and runtime behavior. If the executable layout matches
   an existing profile, add an identity entry pointing to it. If not, add a
   layout with the correct addresses and only the capabilities tested so far.
3. Check a real tactical scene, camera movement and scene transitions against
   mGBA's canonical frame. Text decoding and Armory require separate validation;
   a Japanese-base translation need not share FE8U's encoding or item tables.
4. For native trade, verify process callbacks, units, item slots, UI geometry
   and tutorial behavior. Reuse the existing adapter only if its ABI and policy
   match. Keep unsupported operations disabled.
5. Add ROM-free identity/rejection and capability tests, then run private local
   gameplay checks with scratch saves. Document the exact scope and limits.

A German translation retaining a verified layout would normally need a new
identity entry and validation, not a new renderer or language-specific input
branch. This is an extension path, not a claim that an untested German ROM is
already supported. Stock European and Japanese retail images are not newly
enabled by this change.

## Current additional coverage

The Wolf/Fire Emblem China FE8J translation identified in
[the compatibility evidence](chinese-rom-compatibility.md) enables map and native
trade capabilities. Armory, relocated HUD and FE8U patch probing stay disabled.
The [trade notes](native-trade-mouse.md) describe its private tutorial validation.

`rom_profile_registry` tests compatible aliases and distinct layouts using
synthetic identities, including header, maker, full-hash and null-input
rejection. `fe8j_profile` checks the real registration against synthetic memory
and rejects inventory writes. `native_trade` checks navigation, cancellation,
capability rejection and a synthetic translated panel/item configuration.

LCD output and native-frame window fitting do not depend on any ROM profile.
They can be reviewed and applied independently.
