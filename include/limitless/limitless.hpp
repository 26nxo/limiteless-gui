#pragma once
#include <array>
#include <limitless/animation.hpp>
#include <imgui.h>
#include <map>
#include <string>

namespace limitless {
enum class Category { Combat, Player, Move, Visuals, Misc, Friends, Configs, Settings };
struct ModuleState {
    bool enabled = false;
    ImGuiKey key = ImGuiKey_None;
    std::map<std::string, float> numbers;
    std::map<std::string, bool> switches;
};
struct State {
    bool visible = true;
    Category category = Category::Player;
    std::string page = "Bridge Assist";
    bool enabled = false;
    bool sneak_on_jump = false;
    float edge_offset = 0.0f;
    float unsneak_delay = 150.0f;
    bool holding_blocks = true;
    bool binding = false;
    ImGuiKey key = ImGuiKey_None;
    std::array<char, 128> search{};
    std::map<std::string, ModuleState> modules;
};
class Interface {
  public:
    bool initialize(const char *font_path, const char *icon_font_path = nullptr);
    void render(State &state);

  private:
    ImFont *font_ = nullptr;
    ImFont *icons_ = nullptr;
    std::map<ImGuiID, Animation> tweens_;
    std::map<ImGuiID, Spring> springs_;
    std::map<ImGuiID, float> scroll_targets_;
    std::map<ImGuiID, Animation> scroll_animations_;
    Animation entrance_;
    Animation content_fade_;
    Animation content_slide_;
    Spring category_y_;
    Spring category_width_;
    std::string previous_page_;
    Category previous_category_ = Category::Player;
    bool content_initialized_ = false;
};
}
