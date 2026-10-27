# LCD pixel grid (lcd3x)

On Windows, choose **Settings > LCD pixel grid (lcd3x)**. It is off by default.
**Settings > LCD brightness** offers 1, 2, 4, 6, 8 and 12. Start with **4**,
the reference default; higher values lighten the image and soften the grid.
The switch and brightness persist between launches. This is mask brightness,
not a backlight/exposure control: it does not clip or amplify source colours.

If the image is still too dark, use **Settings > LCD brightness compensation**.
This independent gamma control defaults to **1.4**; try **1.6** for more lift.
**1.0 (original)** reproduces the uncompensated reference. The menu offers
1.0, 1.2, 1.4, 1.6, 1.8 and 2.2, and persists the choice. Unlike reducing mask
strength, compensation raises midtones while retaining the RGB grid. High
values reduce contrast and can make dark colours look washed out. Black stays
black, and highlights are not hard-clipped by a gain multiplier.

The old mask setting cannot restore original brightness: at 12 with rows at
16, the continuous mean mask transmission is `(12/13)*(16/17)`, about 87%.
This new compensation is an FE8-UI extension, not a parameter of RetroArch's
original lcd3x shader.

The SDL command-line frontend also accepts `--lcd`, `--no-lcd` and
`--lcd-brightness 4.0` (1..12, rounded to tenths). Brightness alone does not
enable LCD. `--lcd-gamma 1.4` accepts 1..2.2, rounded to hundredths; use 1 for
the original reference effect. macOS continues to use its existing OpenGL shader presets; this
implementation targets the Windows/Linux SDL renderer.

## Reference and integration decision

Inspected the official libretro files on 2026-10-27:

- [handheld/lcd3x.slangp](https://github.com/libretro/slang-shaders/blob/master/handheld/lcd3x.slangp)
- [handheld/shaders/lcd3x.slang](https://github.com/libretro/slang-shaders/blob/master/handheld/shaders/lcd3x.slang)

The preset contains one pass, nearest-neighbour sampling (`filter_linear0 =
false`) and viewport output scaling. The referenced shader has no additional
includes, LUTs or passes. Gigaherz explicitly dedicates it to the public domain.
Its `brighten_lcd` default is 4 (range 1..12); `brighten_scanlines` defaults
to 16 (range 1..32). No copy of an iOS installation was available, so this
uses the official defaults rather than assuming device-specific overrides.

The SDL2 renderer cannot load a Slang preset directly. Full support would
require a shader compiler/preset loader and another graphics integration.
For this single multiplicative effect, the existing SDL renderer can reproduce
the formula with a cached RGB mask and `SDL_BLENDMODE_MOD` instead:

```
angle = 2*pi * texture_coordinate * source_size
offsets = pi * (1/2, 1/2 - 2/3, 1/2 - 4/3)
row = (16 + sin(angle.y)) / 17
columns.rgb = (brightness + sin(angle.x + offsets.rgb)) / (brightness + 1)
output.rgb = nearest_source.rgb * row * columns.rgb
```

The three phase-shifted columns plus the weaker horizontal modulation produce
the LCD grid. This is not an implementation using only horizontal CRT lines.
No blur, bloom or curvature is added. Optional brightness compensation follows
the grid: `output = pow(nearest_source * mask, 1/gamma)` in normalised RGB.
This is a display tone adjustment, not a claim of accurate LCD colour calibration.

## Rendering and limits

The mask is calculated at physical output pixel centres, relative to the
source canvas, and uploaded only when dimensions or parameters change. The
renderer multiplies it over the nearest-neighbour game image each frame.
The extended map and native game frame share the same source pixel grid.
Letterboxing is outside the mask. Native desktop Armory content bypasses LCD;
the native HUD/cursor overlay is drawn afterwards. SDL logical coordinates
are restored for mouse events after physical-pixel rendering.

For gamma compensation, `pow(source * mask, 1/gamma)` separates into a source
term and a mask term. A cached 256-entry LUT transforms the source into a reused
buffer; the cached mask is transformed on parameter/layout changes. No per-pixel
power function runs each frame. The original canvas, emulation and saves stay
unchanged. Disabling LCD and native desktop content bypass both terms.

Three or more output pixels per game pixel best resolve the pattern; fractional
or smaller scaling can produce aliasing, as with the reference shader. This
does not force integer scaling or crop the map. The 8-bit mask and SDL multiply
rounding can differ slightly from the floating-point Slang result. It follows
the same formula, but is not claimed to be bit-identical on every GPU or to
reproduce an iPhone's physical display. The row parameter remains at 16 in the
menu; LCD brightness adjusts the original shader's `brighten_lcd` parameter.

## Validation

`lcd_grid` checks independent default 3x RGB values, two-axis periodicity,
brightness, fractional scaling and invalid parameters. `lcd_presentation`
exercises the actual SDL backend on a software surface: on/off, cached frames,
brightness changes, overlays, native UI bypass, letterboxing, canvas changes
and restored logical coordinates. `windows_desktop` verifies menu toggles and
preference persistence in an isolated test registry.

The existing `--capture` option saves the source canvas **before** display
effects. For a local diagnostic rendering of that BMP with this same backend:

```
test_lcd_presentation.exe private-input.bmp private-lcd-3x.bmp 140
```

This optional test utility produces a 3x output at default mask brightness;
the optional last argument is gamma in hundredths (100 for the reference).
It also tests gamma identity, monotonicity, black/white endpoints, compositing,
parameter changes, bypass and preference persistence. Keep
ROM-derived inputs and captures local; they are not CI fixtures or repository
assets. CI needs no ROM, BIOS, save or game screenshot.

Validated implementation commit `3578d1f7` on 2026-10-27:

- [Windows build](https://github.com/shichuanliujie9527/FE8-UI/actions/runs/40476355133):
  all 36 tests passed, including both LCD tests and the isolated native menu test.
- [Linux build](https://github.com/shichuanliujie9527/FE8-UI/actions/runs/40476355192)
  and [macOS regression](https://github.com/shichuanliujie9527/FE8-UI/actions/runs/40476355152)
  passed.
- Local Windows run loaded a private checkpoint and presented 60 frames with
  `--lcd --lcd-brightness 4` without a rendering error. This branch alone has
  no Chinese ROM profile; the separate combined playtest branch supplies it.
- The diagnostic renderer passed locally and produced a 3x LCD rendering of a
  private map capture. Visual inspection confirmed the RGB pixel lattice. The
  capture and ROM remain local, and an iOS side-by-side comparison remains open.
