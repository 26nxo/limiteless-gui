#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limitless_gui/limitless.hpp>
#include <imgui_internal.h>
#include <iostream>

namespace {
void require(bool condition, const char *message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
}
int main() {
    limitless_gui::Animation animation;
    animation.reset(0);
    const float midpoint = animation.update(1, 0.06f, 0.12f);
    require(midpoint > 0 && midpoint < 1, "animation snapped instead of interpolating");
    require(animation.update(1, 0.06f, 0.12f) == 1, "animation did not finish at its target");
    const float reverse = animation.update(0, 0.03f, 0.12f);
    require(reverse > 0 && reverse < 1, "animation reversal failed");
    require(animation.update(0, 1.0f, 0.12f) == 0, "animation overshot after a stalled frame");
    limitless_gui::Animation low_fps, high_fps;
    low_fps.reset(0);
    high_fps.reset(0);
    float slow = 0, fast = 0;
    for (int i = 0; i < 3; ++i)
        slow = low_fps.update(1, 1.0f / 30, 0.18f);
    for (int i = 0; i < 12; ++i)
        fast = high_fps.update(1, 1.0f / 120, 0.18f);
    require(std::abs(slow - fast) < 0.0001f, "animation timing depends on frame rate");
    require(limitless_gui::ease(limitless_gui::Easing::ExpoOut, 1) == 1,
            "expo easing does not reach its endpoint");
    require(limitless_gui::ease(limitless_gui::Easing::ExpoOut, 0) == 0, "expo easing does not start at zero");
    limitless_gui::Spring low_rate, high_rate;
    low_rate.reset(0);
    high_rate.reset(0);
    for (int i = 0; i < 18; ++i)
        slow = low_rate.update(1, 1.0f / 60);
    for (int i = 0; i < 72; ++i)
        fast = high_rate.update(1, 1.0f / 240);
    require(std::abs(slow - fast) < 0.001f, "spring timing depends on frame rate");
    for (int i = 0; i < 30; ++i)
        slow = low_rate.update(1, 1.0f / 60);
    require(std::abs(slow - 1) < 0.001f, "spring did not settle");
    low_rate.reset(0);
    const float overshoot = low_rate.update(1, 1.0f / 12);
    require(overshoot > 0 && overshoot < 1.2f, "spring overshot too far");
    ImGui::CreateContext();
    std::error_code fs_error;
    const auto config_dir = std::filesystem::temp_directory_path() / "limitless_gui_tests_configs";
    std::filesystem::remove_all(config_dir, fs_error);
    std::filesystem::create_directories(config_dir, fs_error);
#ifdef _WIN32
    _putenv_s("LIMITLESS_GUI_CONFIG_DIR", config_dir.string().c_str());
#else
    setenv("LIMITLESS_GUI_CONFIG_DIR", config_dir.string().c_str(), 1);
#endif
    limitless_gui::Interface ui;
    limitless_gui::State state;
    require(state.category == limitless_gui::Category::Settings, "settings page did not open by default");
    require(state.settings.show_profile, "profile is hidden by default");
    require(!state.settings.accent_gradient, "gradient accent is enabled by default");
    require(state.settings.menu_key == ImGuiKey_Insert, "default menu key is wrong");
    require(ui.initialize(LIMITLESS_GUI_TEST_FONT), "font initialization failed");
    auto &io = ImGui::GetIO();
    io.DisplaySize = {772, 625};
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    require(io.Fonts->Fonts.Size == 3, "preview fonts were not loaded");
    for (ImWchar glyph : {0xf140, 0xf21d, 0xf70c, 0xf06e, 0xf121, 0xf234, 0xf0c2,
                          0xf1de, 0xf067, 0xf068, 0xf093, 0xf019, 0xf07b, 0xf1f8})
        require(io.Fonts->Fonts[1]->FindGlyphNoFallback(glyph) != nullptr, "font awesome glyph is missing");
    auto frame = [&]() {
        ImGui::NewFrame();
        ui.render(state);
        ImGui::Render();
        require(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0,
                "imgui reported a stack or rendering error");
    };
    auto click = [&](float x, float y) {
        io.AddMousePosEvent(x, y);
        frame();
        io.AddMouseButtonEvent(0, true);
        frame();
        io.AddMouseButtonEvent(0, false);
        frame();
    };
    auto hold = [&](float x, float y) {
        io.AddMousePosEvent(x, y);
        frame();
        io.AddMouseButtonEvent(0, true);
        for (int i = 0; i < 66; ++i)
            frame();
        io.AddMouseButtonEvent(0, false);
        frame();
    };
    frame();
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "renderer produced no geometry");

    click(300, 178);
    click(400, 243);
    require(state.settings.theme == 1, "theme combobox selection failed");
    frame();
    const auto midnight_text = ImGui::GetStyle().Colors[ImGuiCol_Text];
    require(std::abs(midnight_text.x - 205.0f / 255.0f) < 0.01f, "theme palette was not applied");

    click(717, 219);
    require(state.settings.accent_open, "accent color picker did not open");
    bool popup_found = false;
    for (ImGuiWindow *window : ImGui::GetCurrentContext()->Windows)
        if (window->Name && std::strstr(window->Name, "##Popup_")) {
            popup_found = true;
            require((window->Flags & ImGuiWindowFlags_NoMove) != 0, "popup can still be moved");
        }
    require(popup_found, "popup window was not found");
    const auto previous_color = state.settings.accent;
    click(600, 350);
    require(std::abs(state.settings.accent[0] - previous_color[0]) > 0.01f ||
                std::abs(state.settings.accent[1] - previous_color[1]) > 0.01f ||
                std::abs(state.settings.accent[2] - previous_color[2]) > 0.01f,
            "accent color field did not update the color");
    click(716, 380);
    const float alpha = state.settings.accent[3];
    require(alpha > 0.05f && alpha < 0.95f, "accent opacity did not update");
    click(400, 350);
    require(!state.settings.accent_open, "accent color picker did not close");

    click(717, 261);
    require(state.settings.accent_gradient, "gradient accent toggle failed");
    frame();
    click(717, 301);
    require(state.settings.accent_secondary_open, "second accent color picker did not open");
    const auto previous_secondary = state.settings.accent_secondary;
    click(600, 400);
    require(std::abs(state.settings.accent_secondary[0] - previous_secondary[0]) > 0.01f ||
                std::abs(state.settings.accent_secondary[1] - previous_secondary[1]) > 0.01f ||
                std::abs(state.settings.accent_secondary[2] - previous_secondary[2]) > 0.01f,
            "second accent color field did not update the color");
    click(400, 600);
    require(!state.settings.accent_secondary_open, "second accent color picker did not close");
    click(717, 261);
    require(!state.settings.accent_gradient, "gradient accent toggle did not disable");
    require(!state.settings.accent_secondary_open, "hidden accent picker stayed open");

    const float previous_scale = state.settings.scale;
    click(235, 316);
    require(state.settings.scale < previous_scale, "menu scale decrement failed");
    state.settings.scale = 1.0f;
    frame();

    click(717, 351);
    require(!state.settings.animations, "animation toggle failed");
    click(717, 391);
    require(!state.settings.show_profile, "profile toggle failed");
    click(717, 391);
    require(state.settings.show_profile, "profile toggle did not restore");

    click(700, 478);
    require(state.settings.binding_key, "menu key binding did not start");
    io.AddKeyEvent(ImGuiKey_F1, true);
    frame();
    io.AddKeyEvent(ImGuiKey_F1, false);
    frame();
    require(state.settings.menu_key == ImGuiKey_F1, "menu key binding did not update");

    click(717, 517);
    require(!state.settings.close_on_escape, "close on escape toggle failed");
    click(717, 517);
    require(state.settings.close_on_escape, "close on escape toggle did not restore");

    click(80, 366);
    require(state.category == limitless_gui::Category::Configs, "configs category did not open");
    click(400, 172);
    io.AddInputCharactersUTF8("pvp");
    frame();
    require(state.configs.name == "pvp", "config name input failed");
    click(315, 221);
    require(state.configs.entries.size() == 1 && state.configs.entries[0].name == "pvp",
            "config creation failed");
    require(state.configs.name.empty(), "config name was not cleared");
    limitless_gui::load_configs(state.configs);
    require(state.configs.entries.size() == 1, "config file was not saved");
    state.settings.theme = 0;
    click(615, 312);
    require(state.settings.theme == 0, "config applied on a short click");
    hold(615, 312);
    require(state.settings.theme == 1, "config load failed");
    state.settings.theme = 2;
    hold(663, 312);
    limitless_gui::load_configs(state.configs);
    require(state.configs.entries.size() == 1 && state.configs.entries[0].settings.theme == 2,
            "config save was not persisted");
    state.settings.theme = 0;
    hold(615, 312);
    require(state.settings.theme == 2, "config save failed");
    hold(711, 312);
    limitless_gui::load_configs(state.configs);
    require(state.configs.entries.empty(), "config deletion failed");

    click(80, 416);
    require(state.category == limitless_gui::Category::Settings, "settings category did not open");
    io.AddMousePosEvent(80, 68);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(80, 222);
    frame();
    require(state.category == limitless_gui::Category::Visuals, "category drag did not select visuals");
    io.AddMousePosEvent(350, 222);
    frame();
    require(state.category == limitless_gui::Category::Visuals, "drag outside sidebar changed category");
    io.AddMousePosEvent(80, 68);
    frame();
    require(state.category == limitless_gui::Category::Combat, "reverse drag did not select combat");
    io.AddMouseButtonEvent(0, false);
    frame();
    io.DisplaySize = {1158, 938};
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "resized renderer produced no geometry");
    io.AddMousePosEvent(120, 102);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(120, 333);
    frame();
    require(state.category == limitless_gui::Category::Visuals, "category drag failed at 150 percent scale");
    io.AddMouseButtonEvent(0, false);
    frame();

    io.AddKeyEvent(ImGuiKey_Escape, true);
    frame();
    io.AddKeyEvent(ImGuiKey_Escape, false);
    require(!state.visible && ImGui::GetDrawData()->TotalVtxCount == 0, "escape did not hide the menu");
    io.AddKeyEvent(ImGuiKey_F1, true);
    frame();
    io.AddKeyEvent(ImGuiKey_F1, false);
    frame();
    require(state.visible && ImGui::GetDrawData()->TotalVtxCount > 0, "custom menu key did not reopen the menu");
    ImGui::DestroyContext();
    std::cout << "limitless gui interaction checks passed\n";
}
