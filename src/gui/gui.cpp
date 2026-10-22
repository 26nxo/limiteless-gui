#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <gui/gui.hpp>
#include <gui/icons.hpp>
#include <gui/layout.hpp>
#include <string_view>

namespace gui {
namespace {
constexpr ImU32 bg = IM_COL32(22, 22, 22, 255), panel = IM_COL32(17, 17, 17, 255);
constexpr ImU32 text = IM_COL32(197, 197, 197, 255), muted = IM_COL32(111, 111, 111, 255);
constexpr ImU32 pink = IM_COL32(218, 57, 119, 255), border = IM_COL32(40, 40, 40, 255);
struct Canvas {
    ImDrawList *d;
    ImFont *font;
    ImVec2 origin;
    float scale;
    ImFont *icon_font;
    ImU32 tint(ImU32 color) const {
        ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
        value.w *= ImGui::GetStyle().Alpha;
        return ImGui::ColorConvertFloat4ToU32(value);
    }
    ImVec2 p(float x, float y) const {
        return {origin.x + x * scale, origin.y + y * scale};
    }
    void rect(float x, float y, float w, float h, ImU32 c, float r = 0) {
        d->AddRectFilled(p(x, y), p(x + w, y + h), tint(c), r * scale);
    }
    void outline(float x, float y, float w, float h, ImU32 c, float r = 0) {
        d->AddRect(p(x, y), p(x + w, y + h), tint(c), r * scale, 0, scale);
    }
    void line(float x, float y, float xx, float yy, ImU32 c, float width = 2) {
        d->AddLine(p(x, y), p(xx, yy), tint(c), width * scale);
    }
    void circle(float x, float y, float r, ImU32 c, bool fill = true) {
        if (fill)
            d->AddCircleFilled(p(x, y), r * scale, tint(c));
        else
            d->AddCircle(p(x, y), r * scale, tint(c), 0, 2 * scale);
    }
    void label(float x, float y, const char *s, float size = 17, ImU32 c = text) {
        d->AddText(font, size * scale, p(x, y), tint(c), s);
    }
    void right(float x, float y, const char *s, float size = 17, ImU32 c = text) {
        const auto w = font->CalcTextSizeA(size, 10000, 0, s).x;
        label(x - w, y, s, size, c);
    }
    bool hit(const char *id, float x, float y, float w, float h) {
        ImGui::SetCursorScreenPos(p(x, y));
        return ImGui::InvisibleButton(id, {w * scale, h * scale});
    }
    void toggle(const char *id, float x, float y, bool &value) {
        ImGui::PushID(id);
        if (hit(id, x, y, 44, 24))
            value = !value;
        rect(x, y, 44, 24, value ? pink : IM_COL32(27, 27, 27, 255), 12);
        outline(x, y, 44, 24, border, 12);
        circle(x + (value ? 32 : 12), y + 12, 9,
               value ? IM_COL32(33, 26, 29, 255) : IM_COL32(63, 63, 63, 255));
        ImGui::PopID();
    }
    void slider(const char *id, const char *name, float y, float &value, float maximum, float step,
                const char *format) {
        char buffer[48];
        std::snprintf(buffer, sizeof(buffer), format, value);
        label(225, y, name);
        right(739, y, buffer);
        ImGui::PushID(id);
        if (hit("minus", 224, y + 16, 22, 22))
            value = std::max(0.0f, value - step);
        glyph(icons::minus, 235, y + 25, 11, muted);
        if (hit("plus", 718, y + 16, 22, 22))
            value = std::min(maximum, value + step);
        glyph(icons::plus, 730, y + 25, 11, muted);
        hit("track", 249, y + 17, 464, 20);
        if (ImGui::IsItemActive())
            value = std::clamp(
                std::round(((ImGui::GetIO().MousePos.x - origin.x) / scale - 249) / 464 * maximum / step) *
                    step,
                0.0f, maximum);
        rect(249, y + 20, 464, 10, IM_COL32(22, 22, 22, 255), 5);
        const float display = value / maximum;
        if (display > 0) {
            float w = 464 * display;
            rect(249, y + 20, w, 10, pink, 5);
            circle(249 + w - 4, y + 25, 3, IM_COL32(53, 24, 36, 255));
        }
        ImGui::PopID();
    }
    void glyph(const char *icon, float x, float y, float size, ImU32 color) {
        const float width = icon_font->CalcTextSizeA(size, 1000, 0, icon).x;
        d->AddText(icon_font, size * scale, p(x - width / 2, y - size / 2), tint(color), icon);
    }
    void icon(int type, float x, float y, ImU32 c) {
        constexpr const char *symbols[] = {icons::bullseye, icons::street_view, icons::running,
                                           icons::eye,      icons::code,        icons::user_plus,
                                           icons::cloud,    icons::sliders};
        glyph(symbols[type], x, y, 22, c);
    }
};
struct Entry {
    Category category;
    const char *title;
    const char *description;
};
constexpr Entry entries[] = {
    {Category::Combat, "Aim Assist", "Aims at players for you"},
    {Category::Combat, "Auto Clicker", "Clicks while holding LMB"},
    {Category::Combat, "Auto Block", "Blocks after hitting a player"},
    {Category::Combat, "Auto Weapon", "Selects your weapon when aiming at a player"},
    {Category::Combat, "Back Track", "Hit players at their past position"},
    {Category::Combat, "Jump Reset", "Jumps on hit to reduce knockback"},
    {Category::Combat, "Reach", ""},
    {Category::Combat, "Weapons", ""},
    {Category::Player, "Auto Tool", "Picks the best tool when mining"},
    {Category::Player, "Bridge Assist", "Sneaks at block edges"},
    {Category::Player, "Fast Place", "Reduces block placement delay"},
    {Category::Player, "Chest Stealer", "Loots chests automatically"},
    {Category::Player, "Auto Pot", "Throws healing potions on keybind"},
    {Category::Player, "Auto Soup", "Auto-eats soups when low on HP"},
    {Category::Player, "Refill", ""},
    {Category::Move, "WTap", "Resets sprint to increase knockback"},
    {Category::Move, "STap", "Steps back after a hit to increase knockback"},
    {Category::Move, "Auto Sprint", "Always sprint while moving"},
    {Category::Move, "Keep Sprint", "Keep your momentum after attacking a player"},
    {Category::Move, "Fast Stop", "Stop instantly by counter-strafing"},
    {Category::Move, "No Slow", "No slowdown while using items"},
    {Category::Move, "No Jump Delay", ""},
    {Category::Visuals, "Player ESP", ""},
    {Category::Visuals, "Chest ESP", ""},
    {Category::Visuals, "Name Tags", ""},
    {Category::Visuals, "Indicators", ""},
    {Category::Visuals, "Anti Debuff", "Hides blindness and nausea overlays"},
    {Category::Visuals, "Free Look", "Pan the camera without rotating your player"},
    {Category::Visuals, "Array List", "Shows enabled modules on screen"},
    {Category::Visuals, "Watermark", "Shows client branding on screen"},
    {Category::Visuals, "Outlines", "Draws outlines around players through walls"},
    {Category::Visuals, "Trajectories", "Predicts path for bows, pearls and other throwables"}};
}
namespace {
void module_settings(Canvas &c, ModuleState &state, const std::string &name) {
    float y = 180;
    auto section = [&](const char *title) {
        c.label(225, y, title, 13, IM_COL32(88, 88, 88, 255));
        y += 24;
    };
    auto toggle = [&](const char *label, bool initial = false) {
        auto [it, inserted] = state.switches.try_emplace(label, initial);
        c.label(225, y + 3, label, 17, it->second ? text : muted);
        c.toggle(label, 695, y, it->second);
        y += 44;
    };
    auto slider = [&](const char *label, float initial, float maximum, float step, const char *format) {
        auto [it, inserted] = state.numbers.try_emplace(label, initial);
        c.slider(label, label, y, it->second, maximum, step, format);
        y += 52;
    };
    auto choice = [&](const char *label, const char *value) {
        c.rect(225, y, 514, 38, IM_COL32(26, 26, 26, 255), 7);
        c.outline(225, y, 514, 38, border, 7);
        c.label(238, y + 10, label);
        c.right(699, y + 10, value);
        c.glyph(icons::chevron, 723, y + 19, 12, muted);
        if (c.hit(label, 225, y, 514, 38))
            ImGui::OpenPopup(label);
        ImGui::SetNextWindowPos(c.p(225, y + 40));
        ImGui::SetNextWindowSize({514 * c.scale, 0});
        if (ImGui::BeginPopup(label)) {
            ImGui::Selectable(value, true);
            ImGui::EndPopup();
        }
        y += 52;
    };
    section("General");
    if (name == "Aim Assist") {
        choice("Mode", "Multipoint");
        slider("Multipoint offset", 0.79f, 1, 0.01f, "%.2f");
        choice("Targeting", "Single");
        slider("Lock hold", 500, 2000, 10, "%.0f ms");
        choice("Filtering", "FOV");
        slider("Range", 4, 8, 0.1f, "0.0 - %.1f");
        slider("FOV", 60, 180, 1, "%.0f");
        toggle("Visualize FOV");
        toggle("Target line");
        section("Speed");
        slider("Horizontal speed", 75, 100, 1, "%.0f");
        slider("Vertical speed", 0, 100, 1, "%.0f");
        choice("Conditions", "Only clicking");
        section("Behaviour");
        toggle("Through walls");
        toggle("Aim at invisible");
    } else if (name == "Auto Clicker") {
        slider("Max CPS", 21.5f, 30, 0.1f, "%.1f");
        choice("Conditions", "Allow inventory");
        section("Behaviour");
        toggle("Land criticals");
        toggle("Prevent unrefill", true);
        toggle("Start burst");
        toggle("Hit select");
        toggle("Randomization", true);
        if (state.switches["Randomization"]) {
            slider("CPS variation", 2, 10, 0.1f, "%.1f");
            slider("Drop chance", 4, 20, 1, "%.0f%%");
            slider("Drop amount", 3, 10, 0.1f, "%.1f CPS");
            slider("Spike chance", 4, 20, 1, "%.0f%%");
            slider("Spike amount", 2, 10, 0.1f, "%.1f CPS");
        }
    } else if (name == "Chest Stealer") {
        slider("Steal delay", 28, 200, 1, "20 ms - %.0f ms");
        choice("Conditions", "Close on steal, Disable on steal");
    } else if (name == "Weapons") {
        choice("Allowed types", "Swords");
        section("Modules");
        choice("Applies to", "Aim Assist, Auto Clicker");
    } else if (name == "Free Look") {
        auto [it, inserted] = state.switches.try_emplace("Third person", false);
        const char *selected = it->second ? "Third person" : "First person";
        c.rect(225, y, 514, 38, IM_COL32(26, 26, 26, 255), 7);
        c.label(238, y + 10, "Switch to");
        c.right(699, y + 10, selected);
        if (c.hit("perspective", 225, y, 514, 38))
            ImGui::OpenPopup("perspective");
        ImGui::SetNextWindowPos(c.p(225, y + 40));
        ImGui::SetNextWindowSize({514 * c.scale, 0});
        if (ImGui::BeginPopup("perspective")) {
            if (ImGui::Selectable("First person", !it->second))
                it->second = false;
            if (ImGui::Selectable("Third person", it->second))
                it->second = true;
            ImGui::EndPopup();
        }
        y += 52;
    } else if (name == "Indicators") {
        slider("Arrow FOV", 180, 180, 1, "%.0f");
        slider("Arrow opacity", 1, 1, 0.01f, "%.2f");
        toggle("Distance-based color");
        toggle("Behind only", true);
        toggle("Invisible players");
        toggle("Ignore friends", true);
        toggle("Outline", true);
        slider("Outline thickness", 1.5f, 5, 0.1f, "%.1f");
        toggle("Show name");
        toggle("Show distance");
    } else if (name == "Name Tags") {
        toggle("Hide vanilla nametags", true);
        slider("Range", 70, 250, 1, "%.0f");
        slider("Scale", 1, 5, 0.1f, "%.1f");
        toggle("Distance scaling");
        toggle("Background", true);
        toggle("Text shadow", true);
        section("Display");
        toggle("Show health", true);
        choice("Health format", "HP");
    } else if (name == "Player ESP" || name == "Chest ESP") {
        choice("Mode", "2D");
        toggle("Box", true);
        if (name == "Player ESP")
            choice("Box color mode", "Custom");
        toggle("Fill");
        if (name == "Player ESP")
            toggle("Hurt effect");
        else {
            choice("Fill style", "Gradient");
            toggle("Outline", true);
        }
        toggle("Corners", true);
        slider("Corner size", name == "Player ESP" ? 0.31f : 0.10f, 1, 0.01f, "%.2f");
        if (name == "Player ESP") {
            toggle("Health bar", true);
            choice("Health bar style", "HP-based");
            choice("Health bar position", "Left");
            slider("Bar thickness", 1, 5, 1, "%.0f");
            toggle("Animated", true);
            slider("Animation speed", 10, 30, 1, "%.0f");
            toggle("Health number");
            toggle("Absorption");
            toggle("Segments");
            slider("Render distance", 70, 250, 1, "%.0f");
            slider("Fade distance", 1, 10, 0.1f, "%.1f");
        } else {
            section("Ender chests");
            toggle("Show ender chests", true);
        }
    }
    ImGui::SetCursorScreenPos(c.p(225, y));
    ImGui::Dummy({1, 1});
}
}
bool Interface::initialize(const char *path, const char *icon_path) {
    if (!path)
        return false;
    const auto icon_file = icon_path ? std::filesystem::path(icon_path)
                                     : std::filesystem::path(path).parent_path() / "fa-solid-900.ttf";
    if (!std::filesystem::exists(path) || !std::filesystem::exists(icon_file))
        return false;
    auto &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    auto &style = ImGui::GetStyle();
    style.PopupRounding = 8;
    style.FrameRounding = 5;
    style.ScrollbarSize = 5;
    style.Colors[ImGuiCol_PopupBg] = ImColor(32, 32, 32);
    style.Colors[ImGuiCol_Text] = ImColor(197, 197, 197);
    style.Colors[ImGuiCol_Border] = ImColor(49, 49, 49);
    style.Colors[ImGuiCol_Header] = ImColor(55, 29, 40);
    style.Colors[ImGuiCol_HeaderHovered] = ImColor(65, 32, 45);
    style.Colors[ImGuiCol_HeaderActive] = ImColor(80, 35, 53);
    style.Colors[ImGuiCol_ScrollbarBg] = ImColor(17, 17, 17);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(48, 48, 48);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(65, 65, 65);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(80, 80, 80);
    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 2;
    font_ = io.Fonts->AddFontFromFileTTF(path, 20, &config);
    if (!font_)
        font_ = io.Fonts->AddFontDefault();
    static constexpr ImWchar ranges[] = {0xf000, 0xf8ff, 0};
    icons_ = io.Fonts->AddFontFromFileTTF(icon_file.string().c_str(), 22, &config, ranges);
    page_fade_.reset(0);
    io.IniFilename = nullptr;
    return font_ != nullptr && icons_ != nullptr;
}
void Interface::render(State &s) {
    if (!s.binding && !ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Insert, false))
        s.visible = !s.visible;
    if (!s.visible)
        return;
    const auto size = ImGui::GetIO().DisplaySize;
    const float scale = std::min(size.x / 772.0f, size.y / 625.0f);
    const ImVec2 origin{(size.x - 772 * scale) / 2, (size.y - 625 * scale) / 2};
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("preview", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings);
    Canvas c{ImGui::GetWindowDrawList(), font_, origin, scale, icons_};
    c.rect(1, 3, 770, 622, bg, 18);
    c.label(695, 6, "timewarp", 16, muted);
    c.d->PushClipRect(c.p(0, 30), c.p(182, 480), true);
    const ImU32 logo = IM_COL32(48, 48, 48, 255);
    c.rect(91, 77, 171, 43, logo, 14);
    c.rect(91, 382, 171, 43, logo, 14);
    c.d->AddBezierCubic(c.p(133, 110), c.p(133, 210), c.p(222, 235), c.p(222, 269), c.tint(logo), 43 * scale);
    c.d->AddBezierCubic(c.p(222, 267), c.p(222, 312), c.p(133, 350), c.p(133, 391), c.tint(logo), 43 * scale);
    c.d->PopClipRect();
    const char *categories[] = {"Combat", "Player",  "Move",    "Visuals",
                                "Misc",   "Friends", "Configs", "Settings"};
    c.hit("category_navigation", 14, 47, 154, 392);
    const bool dragging_category = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    for (int i = 0; i < 8; ++i) {
        const float y = 47 + i * 49.7f;
        if (dragging_category && ImGui::IsMouseHoveringRect(c.p(14, y), c.p(168, y + 42)) &&
            (static_cast<int>(s.category) != i || ImGui::IsMouseClicked(ImGuiMouseButton_Left))) {
            s.category = static_cast<Category>(i);
            s.page.clear();
            s.binding = false;
        }
    }
    const float selected_y = 47 + static_cast<int>(s.category) * 49.7f;
    const float selected_width =
        font_->CalcTextSizeA(20, 1000, 0, categories[static_cast<int>(s.category)]).x + 57;
    c.rect(14, selected_y, selected_width, 42, pink, 22);
    for (int i = 0; i < 8; ++i) {
        const float y = 47 + i * 49.7f;
        auto color = static_cast<int>(s.category) == i ? IM_COL32(43, 24, 32, 255) : muted;
        c.icon(i, 38, y + 21, color);
        c.label(62, y + 10, categories[i], 20, color);
    }
    c.rect(17, 505, 150, 38, IM_COL32(24, 24, 24, 255), 8);
    c.outline(17, 505, 150, 38, border, 8);
    c.glyph(icons::search, 33, 525, 16, muted);
    ImGui::SetCursorScreenPos(c.p(49, 511));
    ImGui::SetNextItemWidth(112 * scale);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(24, 24, 24, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, muted);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
    ImGui::SetWindowFontScale(0.8f * scale);
    if (ImGui::InputTextWithHint("##search", "Search", s.search.data(), s.search.size()))
        s.page.clear();
    ImGui::SetWindowFontScale(1);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    c.rect(17, 568, 46, 41, IM_COL32(220, 220, 220, 255), 8);
    const ImU32 pixels[8] = {0xffbbb9b7, 0xfff1f1f1, 0xffe6b2ca, 0xffa5a1a0,
                             0xff57575b, 0xffe9e9e9, 0xffb0d7e8, 0xfffbfbfb};
    constexpr int face[7][7] = {{0, 1, 1, 1, 2, 2, 2}, {1, 1, 3, 1, 2, 1, 2}, {1, 1, 3, 1, 2, 1, 2},
                                {1, 1, 4, 1, 1, 4, 1}, {0, 1, 6, 1, 1, 6, 1}, {0, 1, 1, 1, 1, 1, 1},
                                {0, 1, 1, 1, 1, 1, 0}};
    for (int y = 0; y < 7; ++y)
        for (int x = 0; x < 7; ++x)
            c.rect(19 + x * 6, 567 + y * 6, 6, 6, pixels[face[y][x]]);
    c.label(76, 570, "dev", 14);
    c.label(76, 590, "Lifetime", 12, muted);
    c.rect(182, 29, 589, 596, IM_COL32(14, 14, 14, 255), 17);
    c.rect(200, 45, 555, 566, panel, 9);
    if (previous_page_ != s.page || previous_category_ != s.category) {
        page_fade_.reset(0);
        previous_page_ = s.page;
        previous_category_ = s.category;
    }
    const float page_alpha =
        page_fade_.update(1, ImGui::GetIO().DeltaTime, layout::page_transition, false, Easing::Smoothstep);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * page_alpha);
    ImGui::PushID(static_cast<int>(s.category));
    ImGui::PushID(s.page.c_str());
    if (s.page.empty()) {
        ImGui::SetCursorScreenPos(c.p(200, 44));
        ImGui::BeginChild("module_list", {555 * scale, 567 * scale}, ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
        c.d = ImGui::GetWindowDrawList();
        c.origin.y -= ImGui::GetScrollY();
        float y = layout::cards::top;
        for (const auto &entry : entries) {
            if (entry.category != s.category)
                continue;
            std::string title = entry.title, query = s.search.data();
            std::transform(title.begin(), title.end(), title.begin(),
                           [](unsigned char x) { return static_cast<char>(std::tolower(x)); });
            std::transform(query.begin(), query.end(), query.begin(),
                           [](unsigned char x) { return static_cast<char>(std::tolower(x)); });
            if (title.find(query) == std::string::npos)
                continue;
            using namespace layout::cards;
            ImGui::PushID(entry.title);
            bool &enabled =
                std::string_view(entry.title) == "Bridge Assist" ? s.enabled : s.modules[entry.title].enabled;
            c.rect(left, y, width, height, IM_COL32(28, 28, 28, 255), 14);
            c.label(text_left, y + title_top, entry.title, title_size, enabled ? pink : text);
            c.label(text_left, y + description_top, entry.description, description_size, muted);
            if (c.hit("body", left, y, gear_x - 18 - left, height))
                enabled = !enabled;
            if (c.hit("settings", gear_x - 16, y + (height - 32) / 2, 32, 32))
                s.page = entry.title;
            const auto gear_color = ImGui::IsItemHovered() ? text : muted;
            c.glyph(icons::gear, gear_x, y + height / 2, gear_size, gear_color);
            c.toggle("module_enabled", toggle_x, y + (height - 24) / 2, enabled);
            ImGui::PopID();
            y += pitch;
        }
        ImGui::SetCursorScreenPos(c.p(215, y));
        ImGui::Dummy({1, 1});
        ImGui::EndChild();
    } else {
        c.label(224, 60, s.page.c_str(), 28);
        c.circle(722, 75, 17, IM_COL32(29, 29, 29, 255));
        c.glyph(icons::close, 722, 75, 17, muted);
        if (c.hit("close", 704, 57, 36, 36)) {
            s.page.clear();
            s.binding = false;
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopStyleVar();
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }
        c.line(225, 110, 739, 110, IM_COL32(31, 31, 31, 255), 1);
        bool &enabled = s.page == "Bridge Assist" ? s.enabled : s.modules[s.page].enabled;
        ImGuiKey &bound_key = s.page == "Bridge Assist" ? s.key : s.modules[s.page].key;
        c.label(225, 135, "Enabled", 17, muted);
        c.toggle("enabled", 695, 131, enabled);
        c.outline(614, 131, 69, 24, border, 12);
        c.glyph(icons::keyboard, 628, 143, 11, muted);
        const char *key = s.binding                    ? "..."
                          : bound_key == ImGuiKey_None ? "None"
                                                       : ImGui::GetKeyName(bound_key);
        c.label(641, 135, key, 13, muted);
        if (c.hit("binding", 614, 131, 69, 24))
            s.binding = true;
        const bool was_binding = s.binding;
        if (s.binding) {
            for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_MouseLeft; ++k) {
                if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k))) {
                    bound_key = k == ImGuiKey_Escape ? ImGuiKey_None : static_cast<ImGuiKey>(k);
                    s.binding = false;
                    break;
                }
            }
        }
        if (!was_binding && bound_key != ImGuiKey_None && !ImGui::GetIO().WantTextInput &&
            ImGui::IsKeyPressed(bound_key))
            enabled = !enabled;
        if (s.page == "Bridge Assist") {
            c.label(225, 180, "General", 13, IM_COL32(88, 88, 88, 255));
            c.slider("edge", "Edge offset", 203, s.edge_offset, 1.0f, 0.01f, "%.2f");
            c.slider("delay", "Unsneak delay", 255, s.unsneak_delay, 250, 1, "%.0f ms");
            c.label(225, 312, "Sneak on jump", 17, muted);
            c.toggle("jump", 695, 308, s.sneak_on_jump);
            c.label(225, 357, "Conditions", 13, IM_COL32(88, 88, 88, 255));
            c.rect(225, 378, 514, 38, IM_COL32(32, 32, 32, 255), 8);
            c.outline(225, 378, 514, 38, IM_COL32(49, 49, 49, 255), 8);
            c.label(238, 388, "Conditions");
            c.right(698, 388, s.holding_blocks ? "Holding blocks" : "None", 17);
            c.glyph(icons::chevron, 723, 397, 12, text);
            if (c.hit("conditions", 225, 378, 514, 38))
                ImGui::OpenPopup("conditions_popup");
            ImGui::SetNextWindowPos(c.p(225, 421));
            ImGui::SetNextWindowSize({514 * scale, 0});
            ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(32, 32, 32, 255));
            ImGui::PushStyleColor(ImGuiCol_Header, pink);
            if (ImGui::BeginPopup("conditions_popup")) {
                if (ImGui::Selectable("Holding blocks", s.holding_blocks,
                                      ImGuiSelectableFlags_DontClosePopups))
                    s.holding_blocks = !s.holding_blocks;
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(2);
        } else {
            ImGui::SetCursorScreenPos(c.p(210, 174));
            ImGui::PushID(s.page.c_str());
            ImGui::BeginChild("module_settings", {540 * scale, 429 * scale}, ImGuiChildFlags_None,
                              ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
            c.d = ImGui::GetWindowDrawList();
            c.origin.y -= ImGui::GetScrollY();
            module_settings(c, s.modules[s.page], s.page);
            ImGui::EndChild();
            ImGui::PopID();
        }
    }
    ImGui::PopID();
    ImGui::PopID();
    ImGui::PopStyleVar();
    ImGui::End();
    ImGui::PopStyleVar();
}
}
