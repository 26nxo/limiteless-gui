#include <cstdlib>
#include <gui/gui.hpp>
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
    ImGui::CreateContext();
    gui::Interface ui;
    gui::State state;
    require(ui.initialize(GUI_TEST_FONT), "font initialization failed");
    auto &io = ImGui::GetIO();
    io.DisplaySize = {772, 625};
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char *pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    auto frame = [&]() {
        ImGui::NewFrame();
        ui.render(state);
        ImGui::Render();
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
    click(350, 170);
    require(state.page == "Bridge Assist", "module navigation failed");
    require(state.unsneak_delay >= 249 && state.sneak_on_jump, "page navigation lost state");
    click(80, 74);
    require(state.category == gui::Category::Combat && state.page.empty(), "category selection failed");
    click(350, 95);
    require(state.page == "Aim Assist", "aim assist navigation failed");
    click(715, 143);
    require(state.modules["Aim Assist"].enabled && !state.enabled, "module states are not isolated");
    io.DisplaySize = {1158, 938};
    frame();
    require(ImGui::GetDrawData()->TotalVtxCount > 0, "resized renderer produced no geometry");
    ImGui::DestroyContext();
    std::cout << "gui interaction checks passed\n";
}
