#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <limitless_gui/limitless.hpp>
#include <limitless_gui/icons.hpp>
#include <limitless_gui/layout.hpp>

namespace limitless_gui {
namespace {
constexpr ImU32 bg = IM_COL32(22, 22, 22, 255), panel = IM_COL32(17, 17, 17, 255);
constexpr ImU32 text = IM_COL32(197, 197, 197, 255), muted = IM_COL32(111, 111, 111, 255);
constexpr ImU32 pink = IM_COL32(222, 73, 119, 255), border = IM_COL32(40, 40, 40, 255);
ImU32 blend(ImU32 first, ImU32 second, float t) {
    const ImVec4 a = ImGui::ColorConvertU32ToFloat4(first);
    const ImVec4 b = ImGui::ColorConvertU32ToFloat4(second);
    t = std::clamp(t, 0.0f, 1.0f);
    return ImGui::ColorConvertFloat4ToU32({a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                                           a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t});
}
struct Canvas {
    ImDrawList *d;
    ImFont *font;
    ImFont *regular;
    ImVec2 origin;
    float scale;
    ImFont *icon_font;
    std::map<ImGuiID, Animation> &tweens;
    std::map<ImGuiID, Spring> &springs;
    std::map<ImGuiID, float> &scroll_targets;
    std::map<ImGuiID, Animation> &scroll_animations;
    std::map<ImGuiID, float> &color_hues;
    std::map<ImGuiID, std::array<char, 8>> &hex_buffers;
    ImU32 tint(ImU32 color) const {
        ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
        value.w *= ImGui::GetStyle().Alpha;
        return ImGui::ColorConvertFloat4ToU32(value);
    }
    ImVec2 p(float x, float y) const {
        return {origin.x + x * scale, origin.y + y * scale};
    }
    float animate(const char *id, float target, float duration, Easing easing = Easing::CubicOut,
                  bool snap = false) {
        return tweens[ImGui::GetID(id)].update(target, ImGui::GetIO().DeltaTime, duration, snap, easing);
    }
    float spring(const char *id, float target, float stiffness = 560, float damping = 42) {
        return springs[ImGui::GetID(id)].update(target, ImGui::GetIO().DeltaTime, stiffness, damping);
    }
    void smooth_scroll(const char *id) {
        const ImGuiID key = ImGui::GetID(id);
        const float current = ImGui::GetScrollY();
        auto [target, inserted] = scroll_targets.try_emplace(key, current);
        if (inserted)
            scroll_animations[key].reset(current);
        const float maximum = ImGui::GetScrollMaxY();
        auto &io = ImGui::GetIO();
        if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f)
            target->second -= io.MouseWheel * 110.0f * scale;
        target->second = std::clamp(target->second, 0.0f, maximum);
        ImGui::SetScrollY(scroll_animations[key].update(target->second, io.DeltaTime, 0.22f, false,
                                                       Easing::ExpoOut));
    }
    bool begin_popup(const char *id, float x, float y, float width, float height = 0) {
        const bool open = ImGui::IsPopupOpen(id);
        ImGui::PushID(id);
        const float fade = animate("fade", open ? 1.0f : 0.0f, 0.14f, Easing::Smoothstep);
        ImGui::PopID();
        ImGui::SetNextWindowPos(p(x, y + (1.0f - fade) * 6.0f));
        ImGui::SetNextWindowSize({width * scale, height * scale});
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * fade);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {14 * scale, 8 * scale});
        if (ImGui::BeginPopup(id))
            return true;
        ImGui::PopStyleVar(2);
        return false;
    }
    void end_popup() {
        ImGui::EndPopup();
        ImGui::PopStyleVar(2);
    }
    bool combo_option(const char *caption, bool selected) {
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const bool clicked = ImGui::InvisibleButton(caption, {486 * scale, 24 * scale});
        const ImU32 color = selected ? pink : ImGui::IsItemHovered() ? IM_COL32(255, 255, 255, 255) : muted;
        ImGui::GetWindowDrawList()->AddText(regular, 14 * scale, {pos.x, pos.y + 5 * scale}, tint(color), caption);
        return clicked;
    }
    bool combo(const char *id, const char *name, float y, int &selected, const char *const *options,
               int count) {
        ImGui::PushID(id);
        selected = std::clamp(selected, 0, count - 1);
        const bool clicked = hit("field", 225, y, 514, 38);
        const bool open = ImGui::IsPopupOpen(id);
        const float hover = animate("hover", ImGui::IsItemHovered() ? 1.0f : 0.0f, 0.12f,
                                    Easing::Smoothstep);
        rect(225, y, 514, 38, blend(IM_COL32(22, 22, 22, 255), IM_COL32(27, 27, 27, 255), hover), 6);
        outline(225, y, 514, 38, border, 6);
        label(238, y + 11, name, 15, text);
        right(699, y + 11, options[selected], 15, text);
        line(707, y + 8, 707, y + 30, border, 1);
        const float direction = animate("chevron", open ? -1.0f : 1.0f, 0.14f, Easing::Smoothstep);
        line(718, y + 19 - direction * 3, 723, y + 19 + direction * 3, muted, 1.5f);
        line(723, y + 19 + direction * 3, 728, y + 19 - direction * 3, muted, 1.5f);
        if (clicked)
            ImGui::OpenPopup(id);
        ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(16, 16, 16, 255));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(16, 16, 16, 255));
        if (begin_popup(id, 225, y + 40, 514, count * 24.0f + 16)) {
            ImGui::SetWindowFontScale(0.70f * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});
            for (int i = 0; i < count; ++i) {
                if (combo_option(options[i], i == selected)) {
                    selected = i;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::PopStyleVar();
            end_popup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopID();
        return clicked;
    }
    bool button(const char *id, const char *caption, float x, float y, float width = 160) {
        ImGui::PushID(id);
        const bool clicked = hit("button", x, y, width, 28);
        const float hover = animate("hover", ImGui::IsItemHovered() ? 1.0f : 0.0f, 0.12f,
                                    Easing::Smoothstep);
        rect(x, y, width, 28, blend(IM_COL32(34, 34, 34, 255), pink, hover * 0.8f), 6);
        outline(x, y, width, 28, blend(border, pink, hover), 6);
        const float text_width = font->CalcTextSizeA(13, 10000, 0, caption).x;
        label(x + (width - text_width) / 2, y + 7, caption, 13, hover > 0.5f ? text : muted);
        ImGui::PopID();
        return clicked;
    }
    void numeric_value(const char *id, float x, float y, float width, float &value,
                       float minimum, float maximum, const char *format) {
        ImGui::PushID(id);
        auto *storage = ImGui::GetStateStorage();
        const ImGuiID editing_id = ImGui::GetID("editing");
        bool editing = storage->GetBool(editing_id);
        bool focus = false;
        if (!editing) {
            hit("value", x, y, width, 18);
            focus = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
            editing = focus;
        }
        if (editing) {
            ImGui::SetCursorScreenPos(p(x, y));
            ImGui::SetNextItemWidth(width * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {3 * scale, 0});
            if (focus)
                ImGui::SetKeyboardFocusHere();
            const bool submitted = ImGui::InputFloat("##number", &value, 0, 0, format,
                                                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
            if (!std::isfinite(value))
                value = minimum;
            value = std::clamp(value, minimum, maximum);
            if (submitted || ImGui::IsItemDeactivated() || ImGui::IsKeyPressed(ImGuiKey_Escape))
                editing = false;
            ImGui::PopStyleVar();
        } else {
            char buffer[48];
            std::snprintf(buffer, sizeof(buffer), format, value);
            right(x + width, y, buffer, 13, muted);
        }
        storage->SetBool(editing_id, editing);
        ImGui::PopID();
    }
    float color_picker(const char *id, const char *caption, float y, std::array<float, 4> &color,
                       bool &expanded) {
        ImGui::PushID(id);
        label(225, y + 5, caption);
        auto swatch = [&](float x, float top, float size, float radius) {
            const float half = size / 2;
            const ImDrawFlags corners[] = {ImDrawFlags_RoundCornersTopLeft, ImDrawFlags_RoundCornersTopRight,
                                           ImDrawFlags_RoundCornersBottomLeft, ImDrawFlags_RoundCornersBottomRight};
            for (int i = 0; i < 4; ++i) {
                const float left = x + (i % 2) * half, row = top + (i / 2) * half;
                d->AddRectFilled(p(left, row), p(left + half, row + half),
                                 tint(i == 0 || i == 3 ? IM_COL32(95, 95, 95, 255) : IM_COL32(155, 155, 155, 255)),
                                 radius * scale, corners[i]);
            }
            rect(x, top, size, size, ImGui::ColorConvertFloat4ToU32({color[0], color[1], color[2], color[3]}), radius);
        };
        const bool clicked = hit("swatch", 695, y, 44, 24);
        const float hover = animate("hover", ImGui::IsItemHovered() ? 1.0f : 0.0f, 0.12f,
                                    Easing::Smoothstep);
        if (clicked)
            ImGui::OpenPopup("color_editor");
        expanded = ImGui::IsPopupOpen("color_editor");
        rect(695, y, 44, 24, IM_COL32(95, 95, 95, 255), 12);
        rect(695, y, 44, 24, ImGui::ColorConvertFloat4ToU32({color[0], color[1], color[2], color[3]}), 12);
        outline(695, y, 44, 24, blend(border, pink, expanded ? 1.0f : hover * 0.5f), 12);
        if (!expanded) {
            ImGui::PopID();
            return 36;
        }
        const ImVec2 anchor = p(739, y + 28);
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        const float width = 272, height = 310;
        const float left = std::max(8 * scale, std::min(anchor.x - width * scale,
                                                       display.x - (width + 8) * scale));
        const float top = std::max(8 * scale, std::min(anchor.y, display.y - (height + 8) * scale));
        ImGui::SetNextWindowPos({left, top}, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize({width * scale, height * scale});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 16 * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, scale);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(18, 18, 18, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(44, 44, 44, 255));
        if (ImGui::BeginPopup("color_editor")) {
            const ImVec2 origin_backup = origin;
            ImDrawList *draw_backup = d;
            origin = ImGui::GetWindowPos();
            d = ImGui::GetWindowDrawList();
            ImGui::SetWindowFontScale(0.75f * scale);
            auto gradient = [&](float x, float y, float w, float h, float radius, ImU32 start, ImU32 end,
                                bool vertical, ImDrawFlags corners = ImDrawFlags_RoundCornersAll) {
                const int first = d->VtxBuffer.Size;
                d->AddRectFilled(p(x, y), p(x + w, y + h), IM_COL32_WHITE, radius * scale, corners);
                for (int i = first; i < d->VtxBuffer.Size; ++i) {
                    auto &vertex = d->VtxBuffer[i];
                    const float t = vertical ? (vertex.pos.y - p(x, y).y) / (h * scale)
                                             : (vertex.pos.x - p(x, y).x) / (w * scale);
                    const ImU32 coverage = (vertex.col >> IM_COL32_A_SHIFT) & 255;
                    const ImU32 color = tint(blend(start, end, t));
                    vertex.col = (color & ~IM_COL32_A_MASK) |
                                 ((((color >> IM_COL32_A_SHIFT) & 255) * coverage / 255) << IM_COL32_A_SHIFT);
                }
            };
            swatch(16, 16, 26, 8);
            outline(16, 16, 26, 26, IM_COL32(58, 58, 58, 255), 8);
            label(52, 18, caption, 14, text);
            char value[16];
            std::snprintf(value, sizeof(value), "#%02X%02X%02X", static_cast<int>(color[0] * 255 + 0.5f),
                          static_cast<int>(color[1] * 255 + 0.5f), static_cast<int>(color[2] * 255 + 0.5f));
            label(52, 36, value, 11, muted);
            float hue, saturation, brightness;
            ImGui::ColorConvertRGBtoHSV(color[0], color[1], color[2], hue, saturation, brightness);
            const ImGuiID hue_id = ImGui::GetID("hue");
            float &saved_hue = color_hues[hue_id];
            if (saturation > 0.001f)
                saved_hue = hue;
            hue = saved_hue;
            auto hue_color = [&](float h) {
                float r, g, b;
                ImGui::ColorConvertHSVtoRGB(h, 1, 1, r, g, b);
                return ImGui::ColorConvertFloat4ToU32({r, g, b, 1});
            };
            constexpr float field_x = 16, field_y = 58, field_w = 210, field_h = 140;
            gradient(field_x, field_y, field_w, field_h, 10, IM_COL32_WHITE, hue_color(hue), false);
            gradient(field_x, field_y, field_w, field_h, 10, IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 255), true);
            hit("field", field_x, field_y, field_w, field_h);
            if (ImGui::IsItemActive()) {
                const ImVec2 mouse = ImGui::GetIO().MousePos;
                saturation = std::clamp((mouse.x - p(field_x, field_y).x) / (field_w * scale), 0.0f, 1.0f);
                brightness =
                    std::clamp(1.0f - (mouse.y - p(field_x, field_y).y) / (field_h * scale), 0.0f, 1.0f);
                ImGui::ColorConvertHSVtoRGB(hue, saturation, brightness, color[0], color[1], color[2]);
            }
            const float cursor_x = field_x + saturation * field_w;
            const float cursor_y = field_y + (1 - brightness) * field_h;
            circle(cursor_x, cursor_y, 6.4f, IM_COL32(0, 0, 0, 170));
            circle(cursor_x, cursor_y, 4.6f,
                   ImGui::ColorConvertFloat4ToU32({color[0], color[1], color[2], 1}));
            circle(cursor_x, cursor_y, 5.3f, IM_COL32(255, 255, 255, 255), false);
            constexpr float hue_x = 16, hue_y = 214, hue_w = 240, hue_h = 12;
            for (int band = 0; band < 6; ++band) {
                const float from = band / 6.0f, to = (band + 1) / 6.0f;
                gradient(hue_x + hue_w * from, hue_y, hue_w / 6, hue_h, 6,
                         hue_color(from), hue_color(to), false,
                         band == 0 ? ImDrawFlags_RoundCornersLeft : band == 5 ? ImDrawFlags_RoundCornersRight
                                                                                            : ImDrawFlags_RoundCornersNone);
            }
            hit("hue", hue_x, hue_y - 4, hue_w, hue_h + 8);
            if (ImGui::IsItemActive()) {
                hue = std::clamp((ImGui::GetIO().MousePos.x - p(hue_x, hue_y).x) / (hue_w * scale),
                                 0.0f, 1.0f);
                saved_hue = hue;
                ImGui::ColorConvertHSVtoRGB(hue, saturation, brightness, color[0], color[1], color[2]);
            }
            const float hue_cursor = hue_x + hue * hue_w;
            rect(hue_cursor - 3, hue_y - 4, 6, 20, IM_COL32(0, 0, 0, 120), 3);
            rect(hue_cursor - 2, hue_y - 3, 4, 18, IM_COL32(255, 255, 255, 255), 2);
            for (int row = 0; row < 20; ++row)
                for (int col = 0; col < 2; ++col)
                    d->AddRectFilled(p(242 + col * 7, 58 + row * 7), p(249 + col * 7, 65 + row * 7),
                         tint((row + col) % 2 ? IM_COL32(95, 95, 95, 255) : IM_COL32(155, 155, 155, 255)),
                         7 * scale, row == 0 ? (col == 0 ? ImDrawFlags_RoundCornersTopLeft : ImDrawFlags_RoundCornersTopRight)
                         : row == 19 ? (col == 0 ? ImDrawFlags_RoundCornersBottomLeft : ImDrawFlags_RoundCornersBottomRight)
                                     : ImDrawFlags_RoundCornersNone);
            const ImU32 opaque = ImGui::ColorConvertFloat4ToU32({color[0], color[1], color[2], 1});
            const ImU32 transparent = opaque & ~IM_COL32_A_MASK;
            gradient(242, 58, 14, 140, 7, opaque, transparent, true);
            hit("opacity", 238, 58, 22, 140);
            if (ImGui::IsItemActive())
                color[3] = std::clamp(1.0f - (ImGui::GetIO().MousePos.y - p(242, 58).y) / (140 * scale),
                                      0.0f, 1.0f);
            const float alpha_y = 58 + (1 - color[3]) * 140;
            rect(239, alpha_y - 3, 20, 6, IM_COL32(0, 0, 0, 170), 3);
            rect(240, alpha_y - 2, 18, 4, IM_COL32(255, 255, 255, 255), 2);
            label(16, 268, "hex:", 13, muted);
            const ImGuiID hex_id = ImGui::GetID("hex");
            auto &hex = hex_buffers[hex_id];
            if (!ImGui::GetIO().WantTextInput)
                std::snprintf(hex.data(), hex.size(), "%02X%02X%02X",
                              static_cast<int>(color[0] * 255 + 0.5f),
                              static_cast<int>(color[1] * 255 + 0.5f),
                              static_cast<int>(color[2] * 255 + 0.5f));
            ImGui::SetCursorScreenPos(p(52, 260));
            ImGui::SetNextItemWidth(204 * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8 * scale);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {10 * scale, 7 * scale});
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(25, 25, 25, 255));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(32, 32, 32, 255));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(36, 36, 36, 255));
            ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(44, 44, 44, 255));
            ImGui::PushStyleColor(ImGuiCol_Text, text);
            if (ImGui::InputText("##hex", hex.data(), hex.size(),
                                 ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase) &&
                std::char_traits<char>::length(hex.data()) == 6) {
                unsigned rgb = 0;
                if (std::sscanf(hex.data(), "%x", &rgb) == 1) {
                    color[0] = ((rgb >> 16) & 255) / 255.0f;
                    color[1] = ((rgb >> 8) & 255) / 255.0f;
                    color[2] = (rgb & 255) / 255.0f;
                    float h, s, v;
                    ImGui::ColorConvertRGBtoHSV(color[0], color[1], color[2], h, s, v);
                    if (s > 0.001f)
                        saved_hue = h;
                }
            }
            ImGui::PopStyleColor(5);
            ImGui::PopStyleVar(2);
            d = draw_backup;
            origin = origin_backup;
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
        ImGui::PopID();
        return 36;
    }
    void text_input(const char *id, const char *name, float y, std::string &value) {
        label(225, y, name, 14, muted);
        ImGui::PushID(id);
        ImGui::SetCursorScreenPos(p(225, y + 22));
        ImGui::SetNextItemWidth(514 * scale);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(26, 26, 26, 255));
        ImGui::PushStyleColor(ImGuiCol_Text, text);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {9 * scale, 6 * scale});
        ImGui::PushFont(regular);
        std::array<char, 128> buffer{};
        std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());
        if (ImGui::InputText("##value", buffer.data(), buffer.size()))
            value = buffer.data();
        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
        ImGui::PopID();
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
        const float hover = animate("hover", ImGui::IsItemHovered() ? 1.0f : 0.0f, 0.12f,
                                    Easing::Smoothstep);
        const float on = animate("on", value ? 1.0f : 0.0f, 0.16f, Easing::Smoothstep);
        const float knob = spring("knob", value ? 1.0f : 0.0f, 650, 40);
        rect(x, y, 44, 24, blend(blend(IM_COL32(27, 27, 27, 255), IM_COL32(32, 32, 32, 255), hover),
                                 pink, on),
             12);
        outline(x, y, 44, 24, blend(border, IM_COL32(58, 58, 58, 255), hover * 0.7f), 12);
        circle(x + 12 + 20 * knob, y + 12, 9,
               blend(IM_COL32(63, 63, 63, 255), IM_COL32(33, 26, 29, 255), on));
        ImGui::PopID();
    }
    void slider_fill(float x, float center_y, float width, bool range = false) {
        if (width <= 0)
            return;
        const float inset = std::min(5.0f, width / 2);
        const float radius = std::min(3.0f, inset * 0.7f);
        rect(x, center_y - 5, width, 10, pink, 5);
        if (range)
            circle(x + inset, center_y, radius, IM_COL32(53, 24, 36, 255));
        circle(x + width - inset, center_y, radius, IM_COL32(53, 24, 36, 255));
    }
    void slider(const char *id, const char *name, float y, float &value, float maximum, float step,
                const char *format) {
        label(225, y, name);
        ImGui::PushID(id);
        numeric_value("number", 675, y - 2, 64, value, 0, maximum, format);
        if (hit("minus", 224, y + 16, 22, 22))
            value = std::max(0.0f, value - step);
        const float minus_hover = animate("minus_hover", ImGui::IsItemHovered() ? 1.0f : 0.0f, 0.10f,
                                          Easing::Smoothstep);
        glyph(icons::minus, 235, y + 25, 11, blend(muted, text, minus_hover));
        if (hit("plus", 718, y + 16, 22, 22))
            value = std::min(maximum, value + step);
        const float plus_hover = animate("plus_hover", ImGui::IsItemHovered() ? 1.0f : 0.0f, 0.10f,
                                         Easing::Smoothstep);
        glyph(icons::plus, 730, y + 25, 11, blend(muted, text, plus_hover));
        hit("track", 249, y + 17, 464, 20);
        const bool active = ImGui::IsItemActive();
        const bool hovered = ImGui::IsItemHovered();
        if (active)
            value = std::clamp(
                std::round(((ImGui::GetIO().MousePos.x - origin.x) / scale - 249) / 464 * maximum / step) *
                    step,
                0.0f, maximum);
        const float ratio = maximum > 0 ? value / maximum : 0;
        const float display = animate("fill", ratio, 0.18f, Easing::ExpoOut, active);
        const float hover = animate("bar_hover", (active || hovered) ? 1.0f : 0.0f, 0.12f,
                                    Easing::Smoothstep);
        rect(249, y + 20, 464, 10,
             blend(IM_COL32(22, 22, 22, 255), IM_COL32(26, 26, 26, 255), hover), 5);
        slider_fill(249, y + 25, 464 * display);
        ImGui::PopID();
    }
    void glyph(const char *icon, float x, float y, float size, ImU32 color) {
        const float width = icon_font->CalcTextSizeA(size, 1000, 0, icon).x;
        d->AddText(icon_font, size * scale, p(x - width / 2, y - size / 2), tint(color), icon);
    }
    void icon(Category category, float x, float y, ImU32 c) {
        glyph(category == Category::Settings ? icons::sliders : icons::cloud, x, y, 22, c);
    }
};
constexpr const char *themes[] = {"Dark", "Midnight", "Light"};

void settings_page(Canvas &c, SettingsState &state) {
    float y = 135;
    c.combo("theme", "Theme", y, state.theme, themes, 3);
    y += 48;
    c.slider("scale", "Menu scale", y, state.scale, 2, 0.05f, "%.2f");
    y += 52;
    c.label(225, y + 3, "Enable animations", 17, state.animations ? text : muted);
    c.toggle("animations", 695, y, state.animations);
    y += 44;
    y += c.color_picker("accent", "Accent color", y, state.accent, state.accent_open);
    ImGui::SetCursorScreenPos(c.p(225, y));
    ImGui::Dummy({1, 1});
}

void configs_page(Canvas &c, ConfigsState &state) {
    float y = 135;
    c.text_input("config_name", "Config name", y, state.name);
    y += 72;
    if (c.button("create", "Create config", 225, y) && !state.name.empty() &&
        std::find(state.entries.begin(), state.entries.end(), state.name) == state.entries.end()) {
        state.entries.push_back(state.name);
        state.name.clear();
    }
    y += 44;
    for (auto it = state.entries.begin(); it != state.entries.end();) {
        c.label(225, y + 5, it->c_str(), 15);
        if (c.button(it->c_str(), "Remove", 649, y, 90))
            it = state.entries.erase(it);
        else
            ++it;
        y += 40;
    }
    ImGui::SetCursorScreenPos(c.p(225, y));
    ImGui::Dummy({1, 1});
}
} // namespace
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
    const auto regular_file = std::filesystem::path(path).parent_path() / "PreviewSans-Regular.ttf";
    regular_ = std::filesystem::exists(regular_file)
                   ? io.Fonts->AddFontFromFileTTF(regular_file.string().c_str(), 14, &config)
                   : font_;
    tweens_.clear();
    springs_.clear();
    scroll_targets_.clear();
    scroll_animations_.clear();
    color_hues_.clear();
    hex_buffers_.clear();
    entrance_.reset(0);
    content_fade_.reset(1);
    content_slide_.reset(0);
    previous_category_ = Category::Settings;
    content_initialized_ = false;
    io.IniFilename = nullptr;
    return font_ != nullptr && icons_ != nullptr;
}
void Interface::render(State &s) {
    auto &io = ImGui::GetIO();
    const float dt = io.DeltaTime;
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Insert, false))
        s.visible = !s.visible;
    const float menu_alpha = entrance_.update(s.visible ? 1.0f : 0.0f, dt, 0.18f, false,
                                             Easing::Smoothstep);
    if (!s.visible && menu_alpha <= 0.001f)
        return;
    const auto size = io.DisplaySize;
    const float scale = std::min(size.x / 772.0f, size.y / 625.0f);
    const ImVec2 origin{(size.x - 772 * scale) / 2, (size.y - 625 * scale) / 2};
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * menu_alpha);
    ImGui::Begin("preview", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings |
                     (s.visible ? ImGuiWindowFlags_None : ImGuiWindowFlags_NoInputs));
    Canvas c{ImGui::GetWindowDrawList(), font_, regular_, origin, scale, icons_, tweens_, springs_,
             scroll_targets_, scroll_animations_, color_hues_, hex_buffers_};
    c.rect(1, 3, 770, 622, bg, 18);
    c.right(758, 7, "timewarp", 14, muted);
    const ImU32 logo = IM_COL32(48, 48, 48, 255);
    c.glyph("\xef\x89\x94", 89, 586, 66, logo);
    constexpr Category categories[] = {Category::Settings, Category::Configs};
    constexpr const char *names[] = {"Settings", "Configs"};
    constexpr int category_count = 2;
    c.hit("category_navigation", 14, 47, 154, category_count * 49.7f);
    const bool dragging_category = ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    for (int i = 0; i < category_count; ++i) {
        const float y = 47 + i * 49.7f;
        if (dragging_category && ImGui::IsMouseHoveringRect(c.p(14, y), c.p(168, y + 42)) &&
            (static_cast<int>(s.category) != i || ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
            s.category = categories[i];
    }
    const float selected_y = 47 + static_cast<int>(s.category) * 49.7f;
    const float selected_width = font_->CalcTextSizeA(17, 1000, 0, names[static_cast<int>(s.category)]).x + 57;
    const float pill_y = category_y_.update(selected_y, dt, 560, 44);
    const float pill_width = category_width_.update(selected_width, dt, 560, 44);
    c.rect(14, pill_y, pill_width, 42, pink, 22);
    std::array<float, category_count> hover_weights{};
    for (int i = 0; i < category_count; ++i) {
        const float y = 47 + i * 49.7f;
        const bool hovered =
            ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(c.p(14, y), c.p(168, y + 42));
        ImGui::PushID(names[i]);
        hover_weights[i] = c.animate("hover", hovered ? 1.0f : 0.0f, 0.12f, Easing::Smoothstep);
        ImGui::PopID();
    }
    for (int i = 0; i < category_count; ++i) {
        const float y = 47 + i * 49.7f;
        const float selected = std::clamp(1.0f - std::abs(pill_y - y) / 49.7f, 0.0f, 1.0f);
        const ImU32 base = blend(muted, text, hover_weights[i] * 0.35f);
        const auto color = blend(base, IM_COL32(43, 24, 32, 255), selected);
        c.icon(categories[i], 38, y + 21, color);
        c.label(62, y + 12, names[i], 17, color);
    }
    c.label(25, 571, "dev", 13);
    c.label(25, 591, "30 days", 10, muted);
    c.rect(182, 29, 589, 596, IM_COL32(14, 14, 14, 255), 17);
    c.rect(200, 45, 555, 566, panel, 9);
    if (!content_initialized_) {
        previous_category_ = s.category;
        content_initialized_ = true;
        content_fade_.reset(1);
        content_slide_.reset(0);
    } else if (previous_category_ != s.category) {
        const float direction = static_cast<int>(s.category) > static_cast<int>(previous_category_) ? 1.0f : -1.0f;
        content_fade_.reset(0);
        content_slide_.reset(direction * 10.0f);
        previous_category_ = s.category;
    }
    const float content_alpha =
        content_fade_.update(1, dt, layout::page_transition, false, Easing::Smoothstep);
    c.origin.x += content_slide_.update(0, dt, 0.22f, false, Easing::ExpoOut) * scale;
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * content_alpha);
    ImGui::PushID(static_cast<int>(s.category));
    const char *title = s.category == Category::Settings ? "Settings" : "Configs";
    c.label(224, 62, title, 22);
    c.line(225, 110, 739, 110, IM_COL32(31, 31, 31, 255), 1);
    ImGui::SetCursorScreenPos(c.p(210, 124));
    ImGui::BeginChild("direct_page", {540 * scale, 479 * scale}, ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar |
                          ImGuiWindowFlags_NoScrollWithMouse);
    c.d = ImGui::GetWindowDrawList();
    c.smooth_scroll("direct_scroll");
    c.origin.y -= ImGui::GetScrollY();
    if (s.category == Category::Settings)
        settings_page(c, s.settings);
    else
        configs_page(c, s.configs);
    ImGui::EndChild();
    ImGui::PopID();
    ImGui::PopStyleVar();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}
}
