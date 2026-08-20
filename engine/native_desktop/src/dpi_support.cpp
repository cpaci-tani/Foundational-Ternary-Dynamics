#include "native_desktop/dpi_support.h"

namespace ftd::native_desktop {

bool enable_per_monitor_v2_dpi() {
    if (SetProcessDpiAwarenessContext(
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        return true;
    }
    return AreDpiAwarenessContextsEqual(
               GetThreadDpiAwarenessContext(),
               DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) != FALSE;
}

bool apply_dpi_suggested_rect(HWND hwnd, LPARAM lparam) {
    if (!hwnd || !lparam) return false;
    const auto* rect = reinterpret_cast<const RECT*>(lparam);
    return SetWindowPos(hwnd, nullptr, rect->left, rect->top,
                        rect->right - rect->left,
                        rect->bottom - rect->top,
                        SWP_NOACTIVATE | SWP_NOZORDER) != FALSE;
}

}  // namespace ftd::native_desktop
