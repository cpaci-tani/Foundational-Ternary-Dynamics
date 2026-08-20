#pragma once

#include <algorithm>
#include <cstdint>

namespace ftd::native {

// Client-pixel rectangle of the 3D scene inside the top-level HWND
// (SPEC_UI_V2 §3.5 / Phase 1b). Width or height 0 means "use the full
// backbuffer" at the presenter.
struct SceneRect {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

inline bool scene_rect_valid(const SceneRect& r) {
    return r.width > 0 && r.height > 0;
}

inline float scene_aspect(const SceneRect& r) {
    if (!scene_rect_valid(r)) {
        return 1.0f;
    }
    return static_cast<float>(r.width) / static_cast<float>(r.height);
}

inline bool scene_contains_client(const SceneRect& r, int client_x, int client_y) {
    if (!scene_rect_valid(r)) {
        return false;
    }
    return client_x >= r.x && client_y >= r.y
        && client_x < r.x + static_cast<int>(r.width)
        && client_y < r.y + static_cast<int>(r.height);
}

inline void client_to_scene(const SceneRect& r, int client_x, int client_y,
                            int* scene_x, int* scene_y) {
    const int sx = client_x - r.x;
    const int sy = client_y - r.y;
    if (scene_x) *scene_x = sx;
    if (scene_y) *scene_y = sy;
}

inline bool scene_accepts_pointer(const SceneRect& r, int client_x, int client_y,
                                  bool imgui_wants_mouse) {
    return !imgui_wants_mouse && scene_contains_client(r, client_x, client_y);
}

inline bool scene_accepts_keyboard(bool imgui_wants_keyboard, bool win32_edit_focus) {
    return !imgui_wants_keyboard && !win32_edit_focus;
}

inline SceneRect scene_rect_clamped_to(const SceneRect& r, std::uint32_t fb_w,
                                       std::uint32_t fb_h) {
    if (!scene_rect_valid(r) || fb_w == 0 || fb_h == 0) {
        return {0, 0, fb_w, fb_h};
    }
    const int x = std::max(0, r.x);
    const int y = std::max(0, r.y);
    const int w = std::min(static_cast<int>(r.width), static_cast<int>(fb_w) - x);
    const int h = std::min(static_cast<int>(r.height), static_cast<int>(fb_h) - y);
    if (w <= 0 || h <= 0) {
        return {0, 0, fb_w, fb_h};
    }
    return {x, y, static_cast<std::uint32_t>(w), static_cast<std::uint32_t>(h)};
}

}  // namespace ftd::native
