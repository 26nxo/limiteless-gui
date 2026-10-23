#pragma once
#include <array>
#include <limitless_gui/animation.hpp>
#include <imgui.h>
#include <map>
#include <string>
#include <vector>

namespace limitless_gui {
enum class Category { Combat, Player, Move, Visuals, Misc, Friends, Configs, Settings };
struct ModuleState {
    bool enabled = false;
    ImGuiKey key = ImGuiKey_None;
    std::map<std::string, float> numbers;
    std::map<std::string, bool> switches;
    std::map<std::string, int> choices;
    std::map<std::string, std::string> texts;
    std::map<std::string, std::array<float, 2>> ranges;
    std::map<std::string, std::array<float, 4>> colors;
    std::map<std::string, std::vector<bool>> multi_choices;
};
struct State {
    bool visible = true;
    Category category = Category::Player;
    std::string page = "Widget Gallery";
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
    ImFont *regular_ = nullptr;
    ImFont *icons_ = nullptr;
    std::map<ImGuiID, Animation> tweens_;
    std::map<ImGuiID, Spring> springs_;
    std::map<ImGuiID, float> scroll_targets_;
    std::map<ImGuiID, Animation> scroll_animations_;
    std::map<ImGuiID, int> range_handles_;
    std::map<ImGuiID, float> color_hues_;
    std::map<ImGuiID, std::array<char, 8>> hex_buffers_;
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
