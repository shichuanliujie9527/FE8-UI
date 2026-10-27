# Feature review map

All four pull requests target `stephenjayakar/FE8-UI:main` independently. The
fork's default branch continues to follow upstream. This integration branch is
for combined validation, not a fifth all-in-one upstream pull request.

| Pull request | Branch | Scope | Runtime dependency |
| --- | --- | --- | --- |
| [#21](https://github.com/stephenjayakar/FE8-UI/pull/21) | `feat/windows-support` | UTF-8 paths for legacy PNG states; local artifact ignore rules | None of the other PRs |
| [#22](https://github.com/stephenjayakar/FE8-UI/pull/22) | `pr/native-frame-fit` | Complete native-frame presentation, shared pointer/HUD coordinates, Windows 3:2 client sizing | None of the other PRs |
| [#23](https://github.com/stephenjayakar/FE8-UI/pull/23) | `feat/lcd-shader` | Optional lcd3x RGB grid, mask brightness and gamma compensation | None of the other PRs |
| [#24](https://github.com/stephenjayakar/FE8-UI/pull/24) | `pr/rom-profile-registry` | Exact identities, reusable layouts, independent capabilities and native trade input | The trade adapter requires its verified profile, included in this PR |

## Composition

The features are logically independent, but #22 and #23 edit the same SDL
presentation code. When merging both, retain the fitted source rectangle in
the LCD mask dimensions, source sampling, overlay handling and logical mouse
coordinates. The resolved renderer is in `test/pr-integration`, introduced by
merge `e53ba58e`. The native-frame diagnostic must also link the LCD/host-settings
sources and the platform math library; `5ac6b93d` records that test integration.
README, workflow artifact entries and test lists retain additions from every PR.

The combined implementation at `5ac6b93d` passed the
[Windows build](https://github.com/shichuanliujie9527/FE8-UI/actions/runs/40490198029),
[Linux build](https://github.com/shichuanliujie9527/FE8-UI/actions/runs/40490197983)
and [macOS regression](https://github.com/shichuanliujie9527/FE8-UI/actions/runs/40490198100).
Later changes to this branch document the review structure only.

## ROM support boundaries

libmGBA emulation is separate from FE8-specific extension support. Window sizing
and LCD output are independent of ROM language. Extensions require verified
memory layouts, not a language-name switch. See [ROM profiles](rom-profiles.md)
for adding aliases, relocated layouts and capability validation.

The new FE8J registration has local small-map and native trade tutorial evidence.
It does not claim support for all translations, European retail images, FE6/FE7,
every chapter, or translated Armory text. No game data is included in the PRs.

## Earlier development branches

`feat/chinese-rom-compat`, `feat/native-trade-mouse`, `fix/native-frame-fit`
and `test/windows-chinese-lcd` retain their earlier development history.
Continue profile development on `pr/rom-profile-registry`, viewport development
on `pr/native-frame-fit`, and combined testing on `test/pr-integration`.
Preserve individual PR boundaries when adding follow-up fixes.

Repository source, comments, documentation and PR descriptions use English.
Private local launch instructions, ROMs, saves and build products are excluded
from Git. Unicode filenames in tests are deliberate compatibility fixtures,
not localized product text.
