#pragma once

#include "imgui.h"

namespace ftd::native {

// Atlas size in DIPs before dpi_scale (SPEC_UI_V2 §7 / §3.5b).
inline constexpr float kUiFontSizeDip = 15.0f;

ImFont* add_embedded_inter_font(ImGuiIO& io, float size_pixels);

}  // namespace ftd::native
