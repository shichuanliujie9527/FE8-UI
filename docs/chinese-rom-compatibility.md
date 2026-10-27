# Chinese ROM compatibility investigation

This branch adds a rendering-only profile for the exact Sacred Stones Chinese
image with SHA-1 `6643a905be9481c5e1807b2a0198cbb5a8d59398`. Other FE8J images,
including the Sacred Evil hack below, remain unsupported by the extension.
Identification was performed on 2026-10-27 against upstream `ae9d2daf`.
No ROM, BIOS, save or game asset is included.

Boot and a tactical scene were captured locally on Windows after 7,200 frames
using ordinary A/Start input. The SDL audio pipeline reported nonzero samples.
This is an automated smoke test, not a human assessment of audio quality or
a complete playthrough. Extended-map runtime validation is tracked separately
from this baseline. Chinese Armory decoding and inventory transfers remain
disabled; the map-only restriction is enforced by the transaction layer too.

| Local image | Header | Size (bytes) | SHA-1 |
| --- | --- | ---: | --- |
| 圣魔之光石 [狼组&火花] | BE8J | 16777216 | `6643a905be9481c5e1807b2a0198cbb5a8d59398` |
| 圣邪的意志 [火花同人] | BE8J | 16777216 | `5f832ecda378f2dab3b1f877f6ca0a8748487288` |
| 封印之剑 [狼组&火花] | AFEJ | 8388608 | `8bd4fa755bdebc58cc9ce59ed4e9dbb80ea6d29d` |
| 烈火之剑 [狼组&火花] | AE7J | 16777216 | `6555bc2623688111cb8e60b105dfb25bf45eff24` |
| 火焰纹章专属定制无敌版 | AE7E | 31176088 | `d6b883f3ae6fa45803184524d4f508c89248422e` |
| 艾格辛里的哲学完结版 (2024.11.21) | AFEJ | 31130836 | `27f22f2fc0d47b99203808827e739b9e2c9ef50e` |

Filenames are labels supplied locally, not verified patch version numbers.
Headers indicate the apparent base game; hacks can retain or alter them.
Use the full digest to distinguish exact images.

The two `BE8J` images are the initial FE8 compatibility targets. The existing
retail-family detector accepts `BE8E` and uses FE8U memory addresses. Simply
allowing `BE8J` would not establish that its maps, units, item tables, text
encoding or convoy match those addresses. FE6/FE7 images need separate game
support; changing the accepted FE8 header cannot provide it.

## Rendering-only layout

Address candidates were cross-checked against
[FEBuilderGBA's FE8JP definitions](https://github.com/FEBuilderGBA/FEBuilderGBA/blob/master/FEBuilderGBA/ROMFE8JP.cs)
and the [FE GBA Function Library](https://github.com/laqieer/FE_GBA_Function_Library),
including its `symbol files/ghidra/FE8J.txt` and
`data xref/FE8J_func_ref_data.json`. The supplied ROM's literal pointers were
then checked locally. No reference implementation or game bytes are copied.

| Structure | FE8J address | Evidence |
| --- | --- | --- |
| Battle-map state | `0202BCAC` | `MoveCameraByStepMaybe` references |
| Chapter state | `0202BCEC` | FEBuilderGBA chapter-data address |
| Map size and seven map handles | `0202E4D0` through `0202E4EC` | `InitChapterMap` references |
| Base metatile row pointer literal | `085C2E54` | `SyncTilesFromMapBuffer` literal at `08019730`; points to `02032F88` |
| Tileset configuration | `02030B88` | `LoadChapterMap` and `UpdateGameTileGfx` references |
| Blue/red/green units | `0202BE48` / `0202CFB8` / `0202DDC8` | FEBuilderGBA unit-array definitions |
| Active unit pointer | `03004DF0` | FEBuilderGBA and `SetupActiveUnit` references |
| Standing sprite list | `0203A014` | `SMS_DisplayAllFromInfoStructs` references |

The profile requires both the `BE8J`/`01` header and the exact file digest.
The ordinary dimension, pointer, row and canonical-frame visual checks still
apply every frame. Native HUD relocation, combat-panel backdrop freezing,
FE8U HP-bar patch probing and inventory editing are disabled for this profile.
The `fe8j_profile` synthetic test checks identity rejection, map extraction,
invalid row fallback and both inventory transaction entry points without ROMs.

## Reproduce identification on Windows

```powershell
./tools/inspect-rom.ps1 -LiteralPath 'D:/Games/Chinese ROMs' | Format-List
# A single file is also accepted, including names containing [] or Chinese.
./tools/inspect-rom.ps1 -LiteralPath 'D:/Games/Chinese ROMs/game.gba'
```

The tool reads headers and hashes without modifying files or writing reports.
It reports candidates, not gameplay compatibility. A file shorter than the
192-byte GBA header is rejected.

## Validation needed before enabling a profile

`--capture frame.bmp --capture-after 7200 --auto-continue --state-out test.ss`
now also exports a checkpoint at capture time if no validated large map was
found. This works for unknown ROMs and scenes with extensions disabled. Keep
these captures and states in an ignored `build-*` directory. A failed state
write returns a failing process exit status. When a large map is found earlier,
the existing earlier checkpoint is retained.

1. Confirm boot and a playable tactical map through ordinary mGBA emulation,
   using isolated scratch saves rather than the player's existing progress.
2. Verify FE8J map, camera, unit and sprite addresses for each exact image;
   compare reconstructed terrain with the canonical emulator frame before
   enabling extended rendering.
3. Identify the translation's text encoding and tables separately from the
   Windows font renderer. Verify character, class and item labels.
4. Validate inventory structures, restrictions and convoy bounds before any
   write-enabled inventory operation; retain fallback for unverified images.

Windows path handling is developed in `feat/windows-support`; LCD presentation
is reserved for `feat/lcd-shader`. Neither change is included in this branch.
