# timewarp preview

standalone c++20 / dear imgui preview using win32 and direct3d 11. no game integration.

## build

requires windows, cmake 3.24+, and visual studio c++ build tools.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

run `build/Release/timewarp_preview.exe`. keep its `assets` folder alongside it.

use `timewarp_preview.exe --category combat` to open a module list directly for visual comparison.
add `--screenshot capture.bmp` to save the rendered frame and exit.

## module

`include/gui/gui.hpp` exposes the state and renderer. initialize once after creating an imgui context, then call `render` between new frame and render. the module has no platform or graphics api dependency.

set `-DGUI_BUILD_PREVIEW=OFF` to build just the portable library and interaction checks. link the `gui` target from a host with its own imgui backend.

the default bridge assist page follows the supplied 772 × 625 image. click or drag sliders, use plus/minus, toggle switches, select conditions, or click the key binding and press a key (escape clears it). close the page to browse categories; search filters the visible category. hold the left mouse button in the sidebar and drag across categories to switch. click a module card to toggle it; click its gear to open settings.

press insert to show or hide the preview. category content fades back in over 100 ms, matching the transition visible in the 30 fps footage. the category pill jumps directly between rows; switches, sliders, popups and wheel scrolling are immediate. frame-by-frame inspection showed no sliding selector or animated switch travel, so those effects were removed.

card measurements come from the updated showcase at 16.1 seconds, normalized to 772 × 625: 546 × 76 cards, 89-pixel row pitch, 18-pixel titles, 15-pixel descriptions, 19-pixel gears, and separate settings hitboxes. layout constants are in `include/gui/layout.hpp`. sidebar rows begin at 47 pixels with a 49.7-pixel pitch. category movement was checked frame by frame around 17.7 seconds.

font awesome free 6.7.2 solid supplies sidebar and control icons. keep `fa-solid-900.ttf` beside the text font, or pass its path as the second argument to `initialize`.

## fidelity

bridge assist is the primary reference page. video-observed controls for aim assist, auto clicker, chest stealer, weapons, free look, indicators, name tags, and esp are also included. unobserved settings and categories are incomplete; dropdowns on secondary pages only show their observed selection. the text font, avatar, background mark, and animation curves are approximations because source assets were not supplied. this is not a verified pixel-identical reconstruction of every video screen.

dear imgui v1.91.9b is vendored under its mit license. preview sans and font awesome fonts are bundled under their open font licenses in `assets/fonts`.
