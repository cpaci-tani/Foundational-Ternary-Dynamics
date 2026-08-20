#pragma once

#include <array>
#include <string>
#include <vector>

namespace ftd::native {

struct Rgba {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct Ramp {
    std::string name;
    std::vector<Rgba> stops;
    bool cvd_safe = false;
};

struct Theme {
    std::string name;
    Rgba surface_0{};
    Rgba surface_1{};
    Rgba surface_2{};
    Rgba surface_3{};
    Rgba text_primary{};
    Rgba text_secondary{};
    Rgba text_muted{};
    Rgba text_dim{};
    Rgba border{};
    Rgba accent{};
    Rgba status_ok{};
    Rgba status_warn{};
    Rgba status_error{};
    struct Data {
        Ramp ternary;
        std::array<Ramp, 3> field_ramps{};
        std::array<Rgba, 8> chart_series{};
    } data;
    struct Metrics {
        float rounding = 2.0f;
        float padding = 6.0f;
        float spacing = 6.0f;
        float border_size = 1.0f;
        float font_size = 15.0f;
    } metrics;
};

struct ThemeParseResult {
    bool ok = false;
    Theme theme;
    std::string error;
};

Theme make_graphite();
Theme make_contrast();
Theme make_slate();
Theme make_carbon();
Theme builtin_theme_by_name(const std::string& name);

ThemeParseResult parse_theme(const std::string& text);
// Resets ImGuiStyle from Theme, then ScaleAllSizes(dpi_scale). The only
// permitted writer of GetStyle / ImGuiCol_ / ImPlotCol_ (SPEC_UI_V2 §9.3c).
void apply_theme(const Theme& theme, float dpi_scale = 1.0f);

}  // namespace ftd::native
