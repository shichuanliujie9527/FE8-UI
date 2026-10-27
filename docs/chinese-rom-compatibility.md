# Chinese ROM compatibility investigation

Status: identification only. No new ROM profile is enabled by this branch.
These results were obtained by read-only inspection of locally supplied files
on 2026-10-27, against upstream `ae9d2daf`. No ROM, BIOS, save or game asset is
included. Booting, extended rendering, Chinese Armory text and inventory writes
have not yet been validated for these images.

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
