# limitless gui

standalone c++20 / dear imgui menu with a win32 / direct3d 11 preview.

## build

requires windows, cmake 3.24+, and visual studio c++ build tools.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

run `build/Release/limitless_gui.exe` with its `assets` folder alongside it.

## preview

the menu opens on the combat module list. click a gear to open settings, click a switch to toggle it, and drag through sidebar categories to navigate. configs is the only config module, so selecting it opens its page directly instead of a module list. press insert to fade the menu in or out. double-click slider values to edit them.

use `--category player` to select a category, `--page "Aim Assist"` to inspect settings, or `--page "Widget Gallery"` for the control gallery. add `--screenshot capture.bmp` to save a settled frame and exit.

## references

the updated video in `references/` supplies the module layout, labels, sidebar icons, footer, and settings controls. earlier videos provide additional interaction examples. fonts, animation timing, and details obscured by video compression are approximations; this is not a verified pixel-exact reproduction. module switches and values are local preview state, with no game integration.

## embedding

`include/limitless_gui/limitless.hpp` exposes the state and renderer. initialize after creating an imgui context, then call `render` between new frame and render. link `limitless_gui_core`; use `-DLIMITLESS_GUI_BUILD_PREVIEW=OFF` for the portable library and interaction checks.

dear imgui is vendored under its mit license. bundled text and icon fonts include their licenses in `assets/fonts`.
