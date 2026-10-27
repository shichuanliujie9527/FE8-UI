# Windows builds

## Game window and native-frame fitting

The game client area keeps the GBA's 3:2 aspect ratio when resized, snapped or
maximised. Maximise selects the largest 3:2 window that fits the monitor work
area instead of filling a differently shaped screen with letterboxing. Startup
uses a regular window. The ROM library is not subject to this game-window rule.

When extended rendering is unavailable (titles, menus, combat transitions,
unsupported ROMs, or extensions disabled), only the authoritative 240x160 frame
is presented, filling that window. A map of at most 15x10 tiles also uses this
view because its entire map already fits in the native frame. Larger extended
scenes and native Armory retain their full canvas. No native game pixels are
cropped or stretched; dark pixels drawn by the game itself remain unchanged.

The internal map canvas is preserved so a display-mode change does not discard
map caches or change emulation. Pointer coordinates and native cursor/HUD
placement use the same presentation crop. This change targets SDL presentation
and Windows window management; macOS retains its existing presentation.

Tests cover the filled native rectangle, full-map restoration, scaled pointer
mapping, cursor overlays, fractional zoom and hidden Win32 resize/maximise
geometry. The diagnostic utility accepts a private raw canvas capture:

`test_native_frame_presentation.exe input.bmp fitted.bmp`

Keep ROM-derived captures local, like the ROM and its saves.

The Windows x64 version uses the SDL2 game window and FreeType inventory UI.
Double-click the executable to open the Windows ROM library. Native Windows
menus provide settings, save states, and Armory access. OpenGL shader presets
(including CRT) remain available only on macOS.

## Build from a Mac with GitHub Actions

Push these files to your fork on GitHub. The **Windows build** workflow runs
automatically on pushes and pull requests. It compiles and runs the unit tests
on a Windows runner; your Mac needs no Windows SDK or cross-compiler.

1. Open your fork's **Actions** tab and select **Windows build**.
2. Open the completed run, or choose **Run workflow** to start another build
   (GitHub shows this button after the workflow reaches the default branch).
3. Download **fe8-ui-windows-x64** from the run's **Artifacts** section.
4. Extract the ZIP on Windows and launch the executable as shown below.

The same workflow can be started from a Mac with GitHub CLI:

```sh
gh workflow run windows.yml --ref main
gh run list --workflow windows.yml
gh run watch RUN_ID
gh run download RUN_ID --name fe8-ui-windows-x64 --dir fe8-ui-windows
```

Replace `RUN_ID` with the run ID printed by `gh run list`. This is a remote
Windows build, so it needs GitHub access; it does not compile locally on macOS.

## Build locally on Windows

Install Visual Studio 2022 with **Desktop development with C++**, CMake, Git,
and an up-to-date vcpkg. From PowerShell:

```powershell
./scripts/build-windows.ps1
# For a standalone vcpkg installation:
./scripts/build-windows.ps1 -VcpkgRoot C:/src/vcpkg
```

If the Visual Studio bundled vcpkg reports missing downloads, install a current
standalone copy and pass its path:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:/src/vcpkg
C:/src/vcpkg/bootstrap-vcpkg.bat -disableMetrics
./scripts/build-windows.ps1 -VcpkgRoot C:/src/vcpkg
```

The script initializes only the pinned mGBA submodule, installs the pinned
SDL2/zlib/FreeType dependencies, builds with MSVC, runs CTest, and stages the
package under `build-windows/dist` and creates `build-windows/fe8-ui-windows-x64.zip`.
It uses static dependencies and the static
MSVC runtime so the executable does not need separate SDL or Visual C++ runtime
DLLs. First builds require network access and take longer than rebuilds.

## Run

Extract the entire release ZIP, then double-click `bin/fe8-mgba-sdl.exe`.
The **ROM Library** opens without command-line arguments:

- Use **Add ROM...** or drop `.gba` files into the library.
- Select a game and choose **Play**, or double-click its row.
- **Resume State** loads that game's last quick save.
- **Remove entry** removes only the library reference; it does not delete files.
- **Save folder** opens that game's storage directory.

The library remembers imported paths and stays open while a game runs. Games
open in separate windows. Avoid running the same ROM in multiple windows at
once, since they share save files.

The game menu bar provides:

- **File**: Open ROM, ROM Library, Close game.
- **State**: Quick save/load and Save state as/Load state.
- **Settings**: Audio, VSync, extended rendering, mouse controls, fast-forward
  speed, zoom sensitivity, and keyboard/hotkey bindings.
- **Armory**: opens inventory when a supported live FE8 roster is available.

Settings persist for future launches. Changes made in a game apply to that game
immediately; library settings apply to games launched afterward. Windows uses
the SDL renderer and does not currently offer CRT shader presets.

Library launches store each ROM's saves under
`%APPDATA%\FE8 Extended Frontend\<ROM SHA-1>\`: `cartridge.sav`,
`quick-state.ss`, and `startup.log`. Identical ROMs share their storage even
when renamed; different ROMs have separate storage. A same-name `.sav` beside
the ROM is imported on first launch if an isolated cartridge save does not
already exist. Startup failures display an error instead of silently closing.

Direct command-line launch is still available. In PowerShell, from the
extracted package directory:

```powershell
./bin/fe8-mgba-sdl.exe --rom 'C:/Games/fireemblem8.gba' --save 'C:/Games/fireemblem8.sav' --quick-state 'C:/Games/fireemblem8.ss'
```

Use your own ROM. Give each game its own save and quick-state paths. Press
**I** for Armory, **F5/F8** to save/load the configured quick state, and **F6**
to toggle extended rendering. Add `--mute` to disable audio. Run with `--help`
for the complete command-line options.

ROM and save-state paths may contain Chinese characters and spaces, including
older PNG-wrapped mGBA states loaded through the compatibility fallback. The
`state_compat` CTest checks that fallback using a synthetic state at a Unicode
path; no ROM, BIOS or real save is needed.
