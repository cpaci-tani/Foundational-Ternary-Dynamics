#include "native/imgui_font.h"

#if IMGUI_VERSION_NUM < 19150
#error "SPEC_UI_V2 requires Dear ImGui >= 1.91.5 (docking)"
#endif
#ifndef IMGUI_HAS_DOCK
#error "SPEC_UI_V2 requires the Dear ImGui docking branch"
#endif

#include "font_inter_regular.inl"

namespace ftd::native {

ImFont* add_embedded_inter_font(ImGuiIO& io, float size_pixels) {
    return io.Fonts->AddFontFromMemoryCompressedTTF(
        font_inter_regular_compressed_data,
        static_cast<int>(font_inter_regular_compressed_size),
        size_pixels);
}

}  // namespace ftd::native
