# LCD pixel grid (lcd3x)

On Windows, choose **Settings > LCD pixel grid (lcd3x)**. It is off by default.
**Settings > LCD brightness** offers 1, 2, 4, 6, 8 and 12. Start with **4**,
the reference default; higher values lighten the image and soften the grid.
The switch and brightness persist between launches. This is mask brightness,
not a backlight/exposure control: it does not clip or amplify source colours.

The SDL command-line frontend also accepts `--lcd`, `--no-lcd` and
`--lcd-brightness 4.0` (1..12, rounded to tenths). Brightness alone does not
enable LCD. macOS continues to use its existing OpenGL shader presets; this
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
No blur, bloom, curvature or colour correction is added.

## Rendering and limits

The mask is calculated at physical output pixel centres, relative to the
source canvas, and uploaded only when dimensions or parameters change. The
renderer multiplies it over the nearest-neighbour game image each frame.
The extended map and native game frame share the same source pixel grid.
Letterboxing is outside the mask. Native desktop Armory content bypasses LCD;
the native HUD/cursor overlay is drawn afterwards. SDL logical coordinates
are restored for mouse events after physical-pixel rendering.

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
test_lcd_presentation.exe private-input.bmp private-lcd-3x.bmp
```

This optional test utility produces a 3x output at default brightness. Keep
ROM-derived inputs and captures local; they are not CI fixtures or repository
assets. CI needs no ROM, BIOS, save or game screenshot.
