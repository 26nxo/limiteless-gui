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

## module

`include/gui/gui.hpp` exposes the state and renderer. initialize once after creating an imgui context, then call `render` between new frame and render. the module has no platform or graphics api dependency.

set `-DGUI_BUILD_PREVIEW=OFF` to build just the portable library and interaction checks. link the `gui` target from a host with its own imgui backend.

the default bridge assist page follows the supplied 772 × 625 image. click or drag sliders, use plus/minus, toggle switches, select conditions, or click the key binding and press a key (escape clears it). close the page to browse categories; search filters the visible category.

## fidelity

bridge assist is the primary reference page. video-observed controls for aim assist, auto clicker, chest stealer, weapons, free look, indicators, name tags, and esp are also included. unobserved settings and categories are incomplete; dropdowns on secondary pages only show their observed selection. fonts, sidebar icons, avatar, and background mark are approximations because source assets were not supplied. this is not a verified pixel-identical reconstruction of every video screen.

dear imgui v1.91.9b is vendored under its mit license. the inter-derived preview sans font is bundled under its open font license in `assets/fonts`.
