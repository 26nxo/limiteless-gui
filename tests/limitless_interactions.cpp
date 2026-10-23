#include <algorithm>
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
    require(limitless_gui::ease(limitless_gui::Easing::ExpoOut, 1) == 1, "expo easing does not reach its endpoint");
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
    require(state.category == limitless_gui::Category::Combat && state.page.empty(),
            "reference module list did not open by default");
    state.category = limitless_gui::Category::Player;
    state.page = "Widget Gallery";
    require(ui.initialize(LIMITLESS_GUI_TEST_FONT), "font initialization failed");
    auto &io = ImGui::GetIO();
    io.DisplaySize = {772, 625};
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    require(io.Fonts->Fonts.Size == 3, "preview fonts were not loaded");
    for (ImWchar glyph : {0xf140, 0xf21d, 0xf70c, 0xf06e, 0xf121, 0xf234, 0xf0c2, 0xf1de, 0xf002, 0xf013,
                          0xf00d, 0xf11c, 0xf078, 0xf067, 0xf068})
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
    require(state.modules["Widget Gallery"].multi_choices.contains("Target groups"),
            "multi combobox was not initialized");
    require(state.modules["Widget Gallery"].colors.contains("Accent color"),
            "color picker was not initialized");
    require(state.page == "Widget Gallery", "gallery page did not open by default");
    click(715, 143);
    require(state.modules["Widget Gallery"].enabled, "enabled toggle failed");
    click(400, 230);
    frame();
    click(300, 292);
    require(state.modules["Widget Gallery"].choices["Allowed types"] == 1,
            "custom combobox selection failed");
    click(730, 381);
    require(state.modules["Widget Gallery"].numbers["Accent strength"] > 0.72f,
            "custom slider increment failed");
    click(710, 360);
    click(710, 360);
    frame();
    io.AddInputCharactersUTF8("0.35");
    frame();
    io.AddKeyEvent(ImGuiKey_Enter, true);
    frame();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    frame();
    require(std::abs(state.modules["Widget Gallery"].numbers["Accent strength"] - 0.35f) < 0.001f,
            "slider numeric editing failed");
    click(695, 430);
    require(state.modules["Widget Gallery"].switches["Enable animations"],
            "custom toggle interaction failed");
    click(236, 507);
    require(state.modules["Widget Gallery"].switches["Show grid"],
            "checkbox interaction failed");
    click(700, 559);
    require(state.modules["Widget Gallery"].ranges["Visible range"][1] > 75,
            "double slider interaction failed");
    click(710, 580);
    click(710, 580);
    frame();
    io.AddInputCharactersUTF8("60");
    frame();
    io.AddKeyEvent(ImGuiKey_Enter, true);
    frame();
    io.AddKeyEvent(ImGuiKey_Enter, false);
    frame();
    require(state.modules["Widget Gallery"].ranges["Visible range"][1] == 60,
            "range numeric editing failed");
    io.AddMousePosEvent(400, 420);
    frame();
    io.AddMouseWheelEvent(0, -8);
    for (int i = 0; i < 30; ++i)
        frame();
    const auto previous_color = state.modules["Widget Gallery"].colors["Accent color"];
    click(727, 349);
    require(state.modules["Widget Gallery"].switches["Accent color open"],
            "color picker did not open");
    click(600, 430);
    const auto edited_color = state.modules["Widget Gallery"].colors["Accent color"];
    require(std::abs(edited_color[0] - previous_color[0]) > 0.01f ||
                std::abs(edited_color[1] - previous_color[1]) > 0.01f ||
                std::abs(edited_color[2] - previous_color[2]) > 0.01f,
            "color field did not update the accent color");
    click(716, 435);
    const float alpha = state.modules["Widget Gallery"].colors["Accent color"][3];
    require(alpha > 0.05f && alpha < 0.95f, "opacity slider did not update transparency");
    click(600, 430);
    click(600, 527);
    require(std::abs(state.modules["Widget Gallery"].colors["Accent color"][3] - alpha) < 0.001f,
            "color editing reset transparency");
    click(400, 350);
    frame();
    require(!state.modules["Widget Gallery"].switches["Accent color open"],
            "color picker did not close");
    click(722, 75);
    require(state.page.empty(), "close button failed");
    click(659, 85);
    require(state.page == "Fast Use", "player module navigation failed");
    click(80, 74);
    require(state.category == limitless_gui::Category::Combat && state.page.empty(),
            "category selection failed");
    io.AddMousePosEvent(80, 123);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    require(state.category == limitless_gui::Category::Player && state.page.empty(),
            "category did not select on press");
    io.AddMousePosEvent(80, 173);
    frame();
    require(state.category == limitless_gui::Category::Move, "held drag did not select move");
    io.AddMousePosEvent(80, 222);
    frame();
    require(state.category == limitless_gui::Category::Visuals, "held drag did not select visuals");
    io.AddMousePosEvent(350, 222);
    frame();
    require(state.category == limitless_gui::Category::Visuals, "drag outside sidebar changed category");
    io.AddMousePosEvent(80, 74);
    frame();
    require(state.category == limitless_gui::Category::Combat, "reverse drag did not return to combat");
    io.AddMouseButtonEvent(0, false);
    frame();
    require(state.page.empty(), "releasing category drag opened a module");
    click(659, 85);
    require(state.page == "Aim Assist", "combat settings navigation failed");
    frame();
    require(state.modules["Aim Assist"].ranges.contains("Range"), "aim range was not initialized");
    click(715, 143);
    require(state.modules["Aim Assist"].enabled, "aim enabled toggle failed");
    for (const char *page : {"Auto Clicker", "Weapons", "Chest Stealer", "Player ESP", "Chest ESP",
                             "Name Tags", "Trajectories", "Indicators", "Array List", "Piercing",
                             "Friends", "Configs", "Settings"}) {
        state.page = page;
        frame();
        frame();
    }
    state.page.clear();
    io.DisplaySize = {1158, 938};
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "resized renderer produced no geometry");
    io.AddMousePosEvent(120, 185);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(120, 333);
    frame();
    require(state.category == limitless_gui::Category::Visuals, "category drag failed at 150 percent scale");
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
    require(state.visible && ImGui::GetDrawData()->TotalVtxCount > 0, "menu did not reappear");    ImGui::DestroyContext();
    std::cout << "limitless gui widget checks passed\n";
}
