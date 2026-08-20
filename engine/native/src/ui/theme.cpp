#include "ui/theme.h"

#include "imgui.h"
#include "implot.h"

#include <cctype>
#include <sstream>

namespace ftd::native {
namespace {

Rgba rgba(float r, float g, float b, float a = 1.0f) {
    return {r, g, b, a};
}

Rgba hex_rgb(unsigned rgb, float a = 1.0f) {
    return rgba(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f,
                (rgb & 0xFF) / 255.0f, a);
}

ImVec4 vec(const Rgba& c) { return {c.r, c.g, c.b, c.a}; }

Rgba lighten(const Rgba& c, float t) {
    return rgba(c.r + (1.0f - c.r) * t, c.g + (1.0f - c.g) * t,
                c.b + (1.0f - c.b) * t, c.a);
}

void fill_neutral_ramps(Theme& t) {
    t.data.ternary.name = "ternary";
    t.data.ternary.cvd_safe = true;
    t.data.ternary.stops = {hex_rgb(0x3b6ea5), hex_rgb(0x4a4d55), hex_rgb(0xd4884a)};
    t.data.field_ramps[0].name = "scalar";
    t.data.field_ramps[0].stops = {hex_rgb(0x1b2838), hex_rgb(0x5b8db8), hex_rgb(0xe6e8eb)};
    t.data.field_ramps[1].name = "signed";
    t.data.field_ramps[1].stops = {hex_rgb(0x2b3f5c), hex_rgb(0x4a4d55), hex_rgb(0xb8752f)};
    t.data.field_ramps[2].name = "magnitude";
    t.data.field_ramps[2].stops = {hex_rgb(0x17181b), hex_rgb(0x5b8db8), hex_rgb(0xc08a4a)};
    const unsigned series[8] = {0x5b8db8, 0xc08a4a, 0x7ba05b, 0xa5688f,
                                0x4f9ba3, 0xb06a5c, 0x8a7fb0, 0x999da6};
    for (int i = 0; i < 8; ++i) t.data.chart_series[static_cast<std::size_t>(i)] = hex_rgb(series[i]);
}

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

bool parse_hex(const std::string& text, Rgba* out) {
    if (!out || text.empty() || text[0] != '#') return false;
    std::string digits = text.substr(1);
    if (digits.size() != 6 && digits.size() != 8) return false;
    unsigned value = 0;
    for (char c : digits) {
        value <<= 4;
        if (c >= '0' && c <= '9') value |= static_cast<unsigned>(c - '0');
        else if (c >= 'a' && c <= 'f') value |= static_cast<unsigned>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') value |= static_cast<unsigned>(c - 'A' + 10);
        else return false;
    }
    if (digits.size() == 6) {
        *out = hex_rgb(value);
    } else {
        *out = rgba(((value >> 24) & 0xFF) / 255.0f, ((value >> 16) & 0xFF) / 255.0f,
                    ((value >> 8) & 0xFF) / 255.0f, (value & 0xFF) / 255.0f);
    }
    return true;
}

}  // namespace

Theme make_graphite() {
    Theme t;
    t.name = "Graphite";
    t.surface_0 = hex_rgb(0x17181b);
    t.surface_1 = hex_rgb(0x1e2024);
    t.surface_2 = hex_rgb(0x2a2d33);
    t.surface_3 = hex_rgb(0x353940);
    t.text_primary = hex_rgb(0xe6e8eb);
    t.text_secondary = hex_rgb(0xb4b8be);
    t.text_muted = hex_rgb(0x8b9098);
    t.text_dim = hex_rgb(0x5c6168);
    t.border = hex_rgb(0x3a3e45);
    t.accent = hex_rgb(0x5b8db8);
    t.status_ok = hex_rgb(0x7ba05b);
    t.status_warn = hex_rgb(0xc08a4a);
    t.status_error = hex_rgb(0xb06a5c);
    fill_neutral_ramps(t);
    return t;
}

Theme make_contrast() {
    Theme t = make_graphite();
    t.name = "Contrast";
    t.surface_0 = hex_rgb(0x0b0c0e);
    t.text_primary = hex_rgb(0xffffff);
    t.accent = hex_rgb(0x7eb6e0);
    t.border = hex_rgb(0x6a717c);
    return t;
}

Theme make_slate() {
    Theme t = make_graphite();
    t.name = "Slate";
    t.surface_0 = hex_rgb(0x1a222b);
    t.surface_1 = hex_rgb(0x222c37);
    t.accent = hex_rgb(0x6a9cc4);
    return t;
}

Theme make_carbon() {
    Theme t = make_graphite();
    t.name = "Carbon";
    t.surface_0 = hex_rgb(0x121212);
    t.surface_1 = hex_rgb(0x1a1a1a);
    t.accent = hex_rgb(0x8a9aa8);
    return t;
}

Theme builtin_theme_by_name(const std::string& name) {
    if (name == "Contrast") return make_contrast();
    if (name == "Slate") return make_slate();
    if (name == "Carbon") return make_carbon();
    return make_graphite();
}

ThemeParseResult parse_theme(const std::string& text) {
    ThemeParseResult result;
    if (text.empty()) {
        result.error = "empty input";
        return result;
    }
    Theme theme = make_graphite();
    std::istringstream in(text);
    std::string line;
    int seen = 0;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) {
            result.error = "malformed line (missing =): " + line;
            return result;
        }
        const std::string key = trim(line.substr(0, eq));
        const std::string value = trim(line.substr(eq + 1));
        if (key.empty() || value.empty()) {
            result.error = "empty key or value";
            return result;
        }
        ++seen;
        auto set_color = [&](Rgba* dst) {
            if (!parse_hex(value, dst)) {
                result.error = "bad colour: " + value;
                return false;
            }
            return true;
        };
        if (key == "name") theme.name = value;
        else if (key == "surface_0") { if (!set_color(&theme.surface_0)) return result; }
        else if (key == "surface_1") { if (!set_color(&theme.surface_1)) return result; }
        else if (key == "surface_2") { if (!set_color(&theme.surface_2)) return result; }
        else if (key == "surface_3") { if (!set_color(&theme.surface_3)) return result; }
        else if (key == "text_primary") { if (!set_color(&theme.text_primary)) return result; }
        else if (key == "text_secondary") { if (!set_color(&theme.text_secondary)) return result; }
        else if (key == "text_muted") { if (!set_color(&theme.text_muted)) return result; }
        else if (key == "text_dim") { if (!set_color(&theme.text_dim)) return result; }
        else if (key == "border") { if (!set_color(&theme.border)) return result; }
        else if (key == "accent") { if (!set_color(&theme.accent)) return result; }
        else if (key == "status_ok") { if (!set_color(&theme.status_ok)) return result; }
        else if (key == "status_warn") { if (!set_color(&theme.status_warn)) return result; }
        else if (key == "status_error") { if (!set_color(&theme.status_error)) return result; }
        else if (key == "metrics.rounding") theme.metrics.rounding = std::stof(value);
        else if (key == "metrics.padding") theme.metrics.padding = std::stof(value);
        else if (key == "metrics.spacing") theme.metrics.spacing = std::stof(value);
        else if (key == "metrics.border_size") theme.metrics.border_size = std::stof(value);
        else if (key == "metrics.font_size") theme.metrics.font_size = std::stof(value);
        else if (key == "data.ternary" || key.rfind("data.field_ramps.", 0) == 0
                 || key == "data.chart_series") {
            continue;
        } else {
            result.error = "unknown key: " + key;
            return result;
        }
    }
    if (seen == 0) {
        result.error = "empty input";
        return result;
    }
    result.ok = true;
    result.theme = std::move(theme);
    return result;
}

void apply_theme(const Theme& theme, float dpi_scale) {
    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle();
    const float p = theme.metrics.padding;
    const float s = theme.metrics.spacing;
    const float r = theme.metrics.rounding;
    const float b = theme.metrics.border_size;
    style.WindowPadding = {p, p};
    style.FramePadding = {p, p * 0.55f};
    style.CellPadding = {p * 0.65f, p * 0.35f};
    style.ItemSpacing = {s, s * 0.75f};
    style.ItemInnerSpacing = {s * 0.65f, s * 0.65f};
    style.IndentSpacing = s * 2.0f;
    style.ScrollbarSize = s * 2.0f;
    style.GrabMinSize = s * 1.6f;
    style.WindowRounding = r;
    style.ChildRounding = r;
    style.FrameRounding = r;
    style.PopupRounding = r;
    style.ScrollbarRounding = r;
    style.GrabRounding = r;
    style.TabRounding = r;
    style.WindowBorderSize = b;
    style.ChildBorderSize = b;
    style.PopupBorderSize = b;
    style.FrameBorderSize = b;
    style.TabBorderSize = b;

    const ImVec4 s0 = vec(theme.surface_0);
    const ImVec4 s1 = vec(theme.surface_1);
    const ImVec4 s2 = vec(theme.surface_2);
    const ImVec4 s3 = vec(theme.surface_3);
    const ImVec4 s2h = vec(lighten(theme.surface_2, 0.08f));
    const ImVec4 s2a = vec(lighten(theme.surface_2, 0.16f));
    const ImVec4 text = vec(theme.text_primary);
    const ImVec4 muted = vec(theme.text_muted);
    const ImVec4 border = vec(theme.border);
    const ImVec4 accent = vec(theme.accent);
    ImVec4 accent_sel = accent;
    accent_sel.w = 0.35f;
    ImVec4 dim_modal = s3;
    dim_modal.w = 0.55f;

    style.Colors[ImGuiCol_WindowBg] = s0;
    style.Colors[ImGuiCol_DockingEmptyBg] = s0;
    style.Colors[ImGuiCol_ChildBg] = s1;
    style.Colors[ImGuiCol_PopupBg] = s1;
    style.Colors[ImGuiCol_MenuBarBg] = s1;
    style.Colors[ImGuiCol_ScrollbarBg] = s1;
    style.Colors[ImGuiCol_TitleBg] = s1;
    style.Colors[ImGuiCol_TitleBgCollapsed] = s1;
    style.Colors[ImGuiCol_TabUnfocused] = s1;
    style.Colors[ImGuiCol_TableRowBg] = s1;
    style.Colors[ImGuiCol_FrameBg] = s2;
    style.Colors[ImGuiCol_FrameBgHovered] = s2h;
    style.Colors[ImGuiCol_FrameBgActive] = s2a;
    style.Colors[ImGuiCol_Button] = s2;
    style.Colors[ImGuiCol_ButtonHovered] = s2h;
    style.Colors[ImGuiCol_ButtonActive] = s2a;
    style.Colors[ImGuiCol_Header] = s2;
    style.Colors[ImGuiCol_HeaderHovered] = s2h;
    style.Colors[ImGuiCol_HeaderActive] = s2a;
    style.Colors[ImGuiCol_TitleBgActive] = s2;
    style.Colors[ImGuiCol_Tab] = s2;
    style.Colors[ImGuiCol_TabUnfocusedActive] = s2;
    style.Colors[ImGuiCol_TableHeaderBg] = s2;
    style.Colors[ImGuiCol_TableRowBgAlt] = s2;
    style.Colors[ImGuiCol_ScrollbarGrab] = s2;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = s2h;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = s2a;
    style.Colors[ImGuiCol_ResizeGrip] = s2;
    style.Colors[ImGuiCol_ResizeGripHovered] = s2h;
    style.Colors[ImGuiCol_ResizeGripActive] = s2a;
    style.Colors[ImGuiCol_TabSelected] = s3;
    style.Colors[ImGuiCol_NavWindowingDimBg] = dim_modal;
    style.Colors[ImGuiCol_ModalWindowDimBg] = dim_modal;
    style.Colors[ImGuiCol_Text] = text;
    style.Colors[ImGuiCol_TextDisabled] = muted;
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    style.Colors[ImGuiCol_Border] = border;
    style.Colors[ImGuiCol_Separator] = border;
    style.Colors[ImGuiCol_SeparatorHovered] = border;
    style.Colors[ImGuiCol_SeparatorActive] = border;
    style.Colors[ImGuiCol_TableBorderStrong] = border;
    style.Colors[ImGuiCol_TableBorderLight] = border;
    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = accent;
    style.Colors[ImGuiCol_SliderGrabActive] = vec(lighten(theme.accent, 0.16f));
    style.Colors[ImGuiCol_TabHovered] = accent;
    style.Colors[ImGuiCol_DockingPreview] = accent;
    style.Colors[ImGuiCol_TextSelectedBg] = accent_sel;
    style.Colors[ImGuiCol_DragDropTarget] = accent;
    style.Colors[ImGuiCol_NavHighlight] = accent;
    style.Colors[ImGuiCol_NavWindowingHighlight] = accent;

    if (ImPlot::GetCurrentContext() != nullptr) {
        ImPlotStyle& ps = ImPlot::GetStyle();
        ps.Colors[ImPlotCol_PlotBg] = s0;
        ps.Colors[ImPlotCol_FrameBg] = s1;
        ps.Colors[ImPlotCol_AxisText] = vec(theme.text_secondary);
    }
    if (dpi_scale > 0.0f) {
        style.ScaleAllSizes(dpi_scale);
    }
}

}  // namespace ftd::native
