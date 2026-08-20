#pragma once

// Repo-owned ImGui compile-time config (SPEC_UI_V2 §10).
// Included via IMGUI_USER_CONFIG from the unmodified vendored imconfig.h.
// Do not include imgui.h from this file — imgui.h includes this file.

void ftd_imgui_assert_dispatch(const char* expr, const char* file, int line);

#define IM_ASSERT(_EXPR) \
    do { \
        if (!(_EXPR)) { \
            ftd_imgui_assert_dispatch(#_EXPR, __FILE__, __LINE__); \
        } \
    } while (0)
