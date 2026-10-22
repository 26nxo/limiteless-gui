# limitless gui

standalone c++20 / dear imgui preview using win32 and direct3d 11. no game integration.

## build

requires windows, cmake 3.24+, and visual studio c++ build tools.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

run `build/Release/limitless_gui.exe`. keep its `assets` folder alongside it.

use `limitless_gui.exe --category player` to open the widget module list directly for visual comparison.
add `--screenshot capture.bmp` to save the rendered frame and exit.

## module

`include/limitless_gui/limitless.hpp` exposes the state and renderer. initialize once after creating an imgui context, then call `render` between new frame and render. the module has no platform or graphics api dependency.

set `-DLIMITLESS_GUI_BUILD_PREVIEW=OFF` to build just the portable library and interaction checks. link the `limitless_gui_core` target from a host with its own imgui backend.

the default widget gallery follows the supplied 772 × 625 image. it contains custom comboboxes, sliders, toggles, text inputs, buttons, progress indicators, and key binding controls. close the page to browse categories; search filters the visible category. hold the left mouse button in the sidebar and drag across categories to switch.

press insert to show or hide the preview; the menu fades out and stops accepting input while closing. category changes use a spring selector with color crossfades, a 160 ms content fade, and a short directional slide. switches and slider fills ease toward their values, hover states transition smoothly, wheel scrolling eases toward its target, and popups fade and lift into place. dragging the category selector and dragging a slider remain immediate.

card measurements come from the reference showcase, normalized to 772 × 625: 546 × 76 cards, 89-pixel row pitch, 18-pixel titles, 15-pixel descriptions, 19-pixel gears, and separate settings hitboxes. layout constants are in `include/limitless_gui/layout.hpp`. sidebar rows begin at 47 pixels with a 49.7-pixel pitch.

font awesome free 6.7.2 solid supplies sidebar and control icons. keep `fa-solid-900.ttf` beside the text font, or pass its path as the second argument to `initialize`.

## fidelity

the widget gallery is the primary reference page. comboboxes use a dark expanded panel, stacked options, a pink selected option, a separator, and a directional chevron. the text font, avatar, background mark, and animation curves are approximations because source assets were not supplied.

dear imgui v1.91.9b is vendored under its mit license. preview sans and font awesome fonts are bundled under their open font licenses in `assets/fonts`.
