# limitless gui

backend-agnostic c++20 dear imgui menu library intended to be rendered from a host dll.

## layout

- `include/limitless_gui/`: public integration api
- `src/limitless_gui/`: menu implementation
- `resources/`: optional preview fonts and avatar
- `examples/preview/`: win32/direct3d 11 host example
- `tests/`: interaction coverage
- `vendor/imgui/`: vendored dear imgui

## embed in a dll

add this repository with `add_subdirectory`, then link the menu target to the dll target:

```cmake
add_subdirectory(path/to/limitless-gui)
target_link_libraries(your_dll PRIVATE limitless_gui::limitless_gui)
```

the host owns the imgui context, backend initialization, render loop, input delivery, and resource lifetime. initialize the menu after the imgui context exists, load persisted configs if wanted, and render between `ImGui::NewFrame()` and `ImGui::Render()`:

```cpp
limitless_gui::Interface menu;
limitless_gui::State state;

menu.initialize(font_path, icon_font_path);
limitless_gui::load_configs(state.configs);

// after imgui::newframe()
menu.render(state);
// before imgui::render()
```

the library does not create a window, install hooks, initialize a graphics backend, or define `DllMain`. assign `State::avatar` to a valid host-owned `ImTextureID` when an avatar is required.

## build

the default build produces the reusable static library and tests:

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

the standalone preview is optional and only available on windows:

```sh
cmake -S . -B build -DLIMITLESS_GUI_BUILD_EXAMPLE=ON
cmake --build build --config Release
```

run `build/Release/limitless_gui_preview.exe`. its `resources` directory is copied beside the executable automatically.

configs are stored in `%APPDATA%\Limitless\configs`; set `LIMITLESS_GUI_CONFIG_DIR` to override the location. dear imgui is vendored under its mit license. bundled fonts include their licenses in `resources/fonts`.
