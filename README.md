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

the sidebar keeps all categories. modules were removed, so only settings and configs show content; the other categories open empty. settings and configs use the custom combobox, slider, toggle, color picker, text input, and button widgets.

settings changes the theme, accent color and optional gradient accent, menu scale, animations, profile visibility, menu key, and escape behavior immediately. the second accent color picker is shown only while gradient accent is enabled. configs create, load, save, and delete in-memory snapshots of the current settings. the preview loads `assets/steve.bmp` for the profile picture; set `State::avatar` to use another texture.

press the menu key (insert by default) to fade the menu in or out. use `--category combat`, `--category configs`, or another category name to choose the opening page. add `--screenshot capture.bmp` to save a settled frame and exit.

## references

the updated video in `references/` supplies the palette, typography, sidebar, footer, and control styling. removing modules changes the reference layout, so the menu is not a pixel-exact reproduction.

## embedding

`include/limitless_gui/limitless.hpp` exposes the state and renderer. initialize after creating an imgui context, then call `render` between new frame and render. link `limitless_gui_core`; use `-DLIMITLESS_GUI_BUILD_PREVIEW=OFF` for the portable library and interaction checks.

dear imgui is vendored under its mit license. bundled text and icon fonts include their licenses in `assets/fonts`.
