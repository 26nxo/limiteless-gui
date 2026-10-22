#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limitless/limitless.hpp>
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
    limitless::Animation animation;
    animation.reset(0);
    const float midpoint = animation.update(1, 0.06f, 0.12f);
    require(midpoint > 0 && midpoint < 1, "animation snapped instead of interpolating");
    require(animation.update(1, 0.06f, 0.12f) == 1, "animation did not finish at its target");
    const float reverse = animation.update(0, 0.03f, 0.12f);
    require(reverse > 0 && reverse < 1, "animation reversal failed");
    require(animation.update(0, 1.0f, 0.12f) == 0, "animation overshot after a stalled frame");
    limitless::Animation low_fps, high_fps;
    low_fps.reset(0);
    high_fps.reset(0);
    float slow = 0, fast = 0;
    for (int i = 0; i < 3; ++i)
        slow = low_fps.update(1, 1.0f / 30, 0.18f);
    for (int i = 0; i < 12; ++i)
        fast = high_fps.update(1, 1.0f / 120, 0.18f);
    require(std::abs(slow - fast) < 0.0001f, "animation timing depends on frame rate");
    require(limitless::ease(limitless::Easing::ExpoOut, 1) == 1, "expo easing does not reach its endpoint");
    require(limitless::ease(limitless::Easing::ExpoOut, 0) == 0, "expo easing does not start at zero");
    limitless::Spring low_rate, high_rate;
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
    limitless::Interface ui;
    limitless::State state;
    require(ui.initialize(LIMITLESS_TEST_FONT), "font initialization failed");
    auto &io = ImGui::GetIO();
    io.DisplaySize = {772, 625};
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    require(io.Fonts->Fonts.Size == 2, "icon font was not loaded");
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
    click(715, 143);
    require(state.enabled, "enabled toggle failed");
    click(715, 320);
    require(state.sneak_on_jump, "jump toggle failed");
    click(730, 228);
    require(state.edge_offset > 0, "slider increment failed");
    click(235, 228);
    require(state.edge_offset == 0, "slider decrement failed");
    click(249, 280);
    require(state.unsneak_delay == 0, "slider minimum failed");
    click(712, 280);
    require(state.unsneak_delay >= 249, "slider maximum failed");
    click(648, 143);
    require(state.binding, "binding capture failed");
    io.AddKeyEvent(ImGuiKey_K, true);
    frame();
    io.AddKeyEvent(ImGuiKey_K, false);
    frame();
    require(state.key == ImGuiKey_K && !state.binding, "key binding failed");
    require(state.enabled, "binding assignment toggled enabled unexpectedly");
    io.AddKeyEvent(ImGuiKey_K, true);
    frame();
    io.AddKeyEvent(ImGuiKey_K, false);
    frame();
    require(!state.enabled, "bound key did not toggle enabled");
    click(720, 397);
    frame();
    click(280, 438);
    require(!state.holding_blocks, "condition selection failed");
    io.AddKeyEvent(ImGuiKey_Escape, true);
    frame();
    io.AddKeyEvent(ImGuiKey_Escape, false);
    frame();
    click(500, 480);
    click(722, 75);
    require(state.page.empty(), "close button failed");
    click(659, 174);
    require(state.page == "Bridge Assist", "module navigation failed");
    require(state.unsneak_delay >= 249 && state.sneak_on_jump, "page navigation lost state");
    click(80, 74);
    require(state.category == limitless::Category::Combat && state.page.empty(), "category selection failed");
    click(350, 85);
    require(state.modules["Aim Assist"].enabled && state.page.empty(),
            "module body should only toggle enabled");
    click(350, 85);
    require(!state.modules["Aim Assist"].enabled, "module body did not toggle back off");
    click(659, 85);
    require(state.page == "Aim Assist", "aim assist navigation failed");
    click(715, 143);
    require(state.modules["Aim Assist"].enabled && !state.enabled, "module states are not isolated");
    io.AddMousePosEvent(450, 400);
    frame();
    io.AddMouseWheelEvent(0, -2);
    frame();
    frame();
    auto current_scroll = [&]() {
        float scroll = 0;
        for (auto *window : ImGui::GetCurrentContext()->Windows)
            if (window->WasActive && (window->Flags & ImGuiWindowFlags_ChildWindow))
                scroll = std::max(scroll, window->Scroll.y);
        return scroll;
    };
    const float initial_scroll = current_scroll();
    require(initial_scroll > 0, "wheel input did not start scrolling");
    for (int i = 0; i < 20; ++i)
        frame();
    const float settled_scroll = current_scroll();
    require(settled_scroll > initial_scroll, "wheel scrolling did not animate smoothly");
    require(settled_scroll > 0, "wheel scrolling failed");
    io.AddMousePosEvent(80, 123);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    require(state.category == limitless::Category::Player && state.page.empty(),
            "category did not select on press");
    io.AddMousePosEvent(80, 173);
    frame();
    require(state.category == limitless::Category::Move, "held drag did not select move");
    io.AddMousePosEvent(80, 222);
    frame();
    require(state.category == limitless::Category::Visuals, "held drag did not select visuals");
    io.AddMousePosEvent(350, 222);
    frame();
    require(state.category == limitless::Category::Visuals, "drag outside sidebar changed category");
    io.AddMousePosEvent(80, 74);
    frame();
    require(state.category == limitless::Category::Combat, "reverse drag did not return to combat");
    io.AddMouseButtonEvent(0, false);
    frame();
    require(state.page.empty(), "releasing category drag opened a module");
    click(659, 85);
    io.AddMousePosEvent(710, 143);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(80, 222);
    frame();
    require(state.category == limitless::Category::Combat, "sidebar stole a drag begun on a module control");
    io.AddMouseButtonEvent(0, false);
    frame();
    io.DisplaySize = {1158, 938};
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "resized renderer produced no geometry");
    io.AddMousePosEvent(120, 185);
    frame();
    io.AddMouseButtonEvent(0, true);
    frame();
    io.AddMousePosEvent(120, 333);
    frame();
    require(state.category == limitless::Category::Visuals, "category drag failed at 150 percent scale");
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
    std::cout << "gui interaction checks passed\n";
}
