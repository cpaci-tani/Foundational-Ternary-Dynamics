// app/app_input.cpp — Win32 window procedure + input helpers (see app/app_input.h).

#include "app/app_input.h"

#include "app/app_context.h"        // AppContext, nudge_last_sheet
#include "native/dpi_support.h"     // apply_dpi_suggested_rect
#include "native/scene_rect.h"      // scene_contains_client

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace ftd::native::app {
// Signed LPARAM coordinate extractors (would come from <windowsx.h>, which we
// cannot include — see the include block above).
inline int lparam_x(LPARAM lp) { return static_cast<int>(static_cast<short>(LOWORD(lp))); }
inline int lparam_y(LPARAM lp) { return static_cast<int>(static_cast<short>(HIWORD(lp))); }

// Max pointer travel (client px) between button-down and button-up that still
// counts as a CLICK (→ scene pick) rather than a DRAG (→ camera orbit).
constexpr int kClickSlop = 4;

int rml_key_modifiers() {
    int m = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) m |= Rml::Input::KM_CTRL;
    if (GetKeyState(VK_SHIFT) & 0x8000) m |= Rml::Input::KM_SHIFT;
    if (GetKeyState(VK_MENU) & 0x8000) m |= Rml::Input::KM_ALT;
    return m;
}

// True when the given client point is inside the laid-out #viewport hole, i.e.
// the pointer is over the 3D scene and should drive the camera rather than the
// (transparent) RML element that marks the hole.
bool over_viewport(const AppContext* app, int x, int y) {
    return ftd::native::scene_contains_client(app->viewport_rect, x, y);
}

AppContext* app_from_hwnd(HWND hwnd) {
    return reinterpret_cast<AppContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    AppContext* app = app_from_hwnd(hwnd);
    Rml::Context* ctx = app ? app->context : nullptr;
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            ftd::native::apply_dpi_suggested_rect(hwnd, lparam);
            return 0;
        case WM_MOUSEMOVE: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseMove(x, y, rml_key_modifiers());
            if (app->dragging) {
                app->camera->yaw += (x - app->last.x) * 0.01f;
                app->camera->pitch += (y - app->last.y) * 0.01f;
                app->camera->pitch = std::max(-1.4f, std::min(1.4f, app->camera->pitch));
                app->last = {x, y};
                // Once the pointer leaves the slop box this press is an orbit
                // drag, not a click — suppress the pick on release.
                if (std::abs(x - app->press_pt.x) > kClickSlop
                    || std::abs(y - app->press_pt.y) > kClickSlop)
                    app->drag_moved = true;
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseButtonDown(0, rml_key_modifiers());
            if (over_viewport(app, x, y)) {
                app->dragging = true;
                app->last = {x, y};
                app->press_pt = {x, y};
                app->press_in_viewport = true;
                app->drag_moved = false;
                SetCapture(hwnd);
            } else {
                app->press_in_viewport = false;
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseButtonUp(0, rml_key_modifiers());
            app->dragging = false;
            if (GetCapture() == hwnd) ReleaseCapture();
            // A press+release inside the viewport with negligible travel is a
            // CLICK → request a scene pick. The GUI loop (which owns the frame,
            // camera, and viewport rect) unprojects + picks; wnd_proc only flags
            // it. A drag (camera already orbited) is ignored here.
            if (app->press_in_viewport && !app->drag_moved && over_viewport(app, x, y)) {
                app->pick_pending = true;
                app->pick_x = x;
                app->pick_y = y;
            }
            app->press_in_viewport = false;
            return 0;
        }
        case WM_RBUTTONDOWN:
            if (ctx) ctx->ProcessMouseButtonDown(1, rml_key_modifiers());
            return 0;
        case WM_RBUTTONUP:
            if (ctx) ctx->ProcessMouseButtonUp(1, rml_key_modifiers());
            return 0;
        case WM_CAPTURECHANGED:
        case WM_KILLFOCUS:
            if (app) app->dragging = false;
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        case WM_MOUSEWHEEL: {
            POINT pt{lparam_x(lparam), lparam_y(lparam)};
            ScreenToClient(hwnd, &pt);
            const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            const bool shift = (GET_KEYSTATE_WPARAM(wparam) & MK_SHIFT) != 0;
            if (over_viewport(app, pt.x, pt.y)) {
                if (shift) {
                    // Shift+wheel over the scene sweeps the most-recently active
                    // sheet up/down through the lattice (tactile height control).
                    // Plain wheel keeps the camera zoom below — so this does NOT
                    // break the existing orbit-camera controls.
                    nudge_last_sheet(app, delta > 0 ? 0.03f : -0.03f);
                } else {
                    app->camera->distance *= (delta > 0) ? 0.9f : 1.1f;
                    app->camera->distance = std::max(4.0f, std::min(512.0f, app->camera->distance));
                }
            } else if (ctx) {
                ctx->ProcessMouseWheel(Rml::Vector2f(0.0f, delta > 0 ? -1.0f : 1.0f),
                                       rml_key_modifiers());
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

}  // namespace ftd::native::app