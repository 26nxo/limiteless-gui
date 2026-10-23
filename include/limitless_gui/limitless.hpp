#pragma once
#include <array>
#include <limitless_gui/animation.hpp>
#include <imgui.h>
#include <map>
#include <string>
#include <vector>

namespace limitless_gui {
enum class Category { Settings, Configs };

struct SettingsState {
    int theme = 0;
    float scale = 1.0f;
    bool animations = true;
    std::array<float, 4> accent{0.92f, 0.25f, 0.47f, 1.0f};
    bool accent_open = false;
};

struct ConfigsState {
    std::string name;
    std::vector<std::string> entries;
};

struct State {
    bool visible = true;
    Category category = Category::Settings;
    SettingsState settings;
    ConfigsState configs;
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
    std::map<ImGuiID, float> color_hues_;
    std::map<ImGuiID, std::array<char, 8>> hex_buffers_;
    Animation entrance_;
    Animation content_fade_;
    Animation content_slide_;
    Spring category_y_;
    Spring category_width_;
    Category previous_category_ = Category::Settings;
    bool content_initialized_ = false;
};
} // namespace limitless_gui
