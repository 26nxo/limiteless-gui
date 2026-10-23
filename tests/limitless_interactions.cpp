#include <cmath>
#include <cstdlib>
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
    limitless_gui::Interface ui;
    limitless_gui::State state;
    require(state.category == limitless_gui::Category::Settings, "settings page did not open by default");
    require(ui.initialize(LIMITLESS_GUI_TEST_FONT), "font initialization failed");
    auto &io = ImGui::GetIO();
    io.DisplaySize = {772, 625};
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    require(io.Fonts->Fonts.Size == 3, "preview fonts were not loaded");
    for (ImWchar glyph : {0xf0c2, 0xf1de, 0xf067, 0xf068})
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
    frame();
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "renderer produced no geometry");
    click(400, 154);
    click(300, 219);
    require(state.settings.theme == 1, "theme combobox selection failed");
    const float scale = state.settings.scale;
    click(730, 210);
    require(state.settings.scale > scale, "settings slider increment failed");
    click(717, 247);
    require(!state.settings.animations, "settings toggle failed");
    click(717, 291);
    require(state.settings.accent_open, "accent color picker did not open");
    const auto previous_color = state.settings.accent;
    click(600, 430);
    require(std::abs(state.settings.accent[0] - previous_color[0]) > 0.01f ||
                std::abs(state.settings.accent[1] - previous_color[1]) > 0.01f ||
                std::abs(state.settings.accent[2] - previous_color[2]) > 0.01f,
            "accent color field did not update the color");
    click(716, 435);
    const float alpha = state.settings.accent[3];
    require(alpha > 0.05f && alpha < 0.95f, "accent opacity did not update");
    click(400, 350);
    require(!state.settings.accent_open, "accent color picker did not close");
    click(80, 118);
    require(state.category == limitless_gui::Category::Configs, "configs category did not open");
    click(400, 170);
    io.AddInputCharactersUTF8("pvp");
    frame();
    require(state.configs.name == "pvp", "config name input failed");
    click(300, 221);
    require(state.configs.entries.size() == 1 && state.configs.entries[0] == "pvp",
            "config creation failed");
    require(state.configs.name.empty(), "config name was not cleared");
    click(690, 265);
    require(state.configs.entries.empty(), "config removal failed");
    click(80, 68);
    require(state.category == limitless_gui::Category::Settings, "settings category did not open");
    io.AddMousePosEvent(80, 68);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(80, 118);
    frame();
    require(state.category == limitless_gui::Category::Configs, "category drag did not select configs");
    io.AddMousePosEvent(350, 118);
    frame();
    require(state.category == limitless_gui::Category::Configs, "drag outside sidebar changed category");
    io.AddMousePosEvent(80, 68);
    frame();
    require(state.category == limitless_gui::Category::Settings, "reverse drag did not select settings");
    io.AddMouseButtonEvent(0, false);
    frame();
    io.DisplaySize = {1158, 938};
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "resized renderer produced no geometry");
    io.AddMousePosEvent(120, 176);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(120, 102);
    frame();
    require(state.category == limitless_gui::Category::Settings, "category drag failed at 150 percent scale");
    io.AddMouseButtonEvent(0, false);
    frame();
    io.AddKeyEvent(ImGuiKey_Insert, true);
    frame();
    io.AddKeyEvent(ImGuiKey_Insert, false);
    require(!state.visible && ImGui::GetDrawData()->TotalVtxCount > 0, "menu did not fade out");
    for (int i = 0; i < 20; ++i)
        frame();
    require(!state.visible && ImGui::GetDrawData()->TotalVtxCount == 0, "menu did not hide");
    io.AddKeyEvent(ImGuiKey_Insert, true);
    frame();
    io.AddKeyEvent(ImGuiKey_Insert, false);
    for (int i = 0; i < 20; ++i)
        frame();
    require(state.visible && ImGui::GetDrawData()->TotalVtxCount > 0, "menu did not reappear");
    ImGui::DestroyContext();
    std::cout << "limitless gui interaction checks passed\n";
}
