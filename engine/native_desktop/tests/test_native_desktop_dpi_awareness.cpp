#include "native_desktop/dpi_support.h"
#include "ftd/test_telemetry.h"

#include <windows.h>

namespace {

LRESULT CALLBACK dpi_test_proc(HWND hwnd, UINT message,
                               WPARAM wparam, LPARAM lparam) {
    if (message == WM_DPICHANGED) {
        ftd::native_desktop::apply_dpi_suggested_rect(hwnd, lparam);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

}  // namespace

int main() {
    ftd::test::init("test_native_desktop_dpi_awareness");
    ftd::test::check("per-monitor-V2 awareness enabled before window creation",
                     ftd::native_desktop::enable_per_monitor_v2_dpi());

    WNDCLASSW wc{};
    wc.lpfnWndProc = dpi_test_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdDpiAwarenessTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"",
                                WS_OVERLAPPEDWINDOW,
                                1, 1, 64, 64, nullptr, nullptr,
                                wc.hInstance, nullptr);
    ftd::test::check("hidden top-level window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    RECT suggested{40, 50, 440, 350};
    SendMessageW(hwnd, WM_DPICHANGED, MAKELONG(144, 144),
                 reinterpret_cast<LPARAM>(&suggested));
    RECT actual{};
    GetWindowRect(hwnd, &actual);
    ftd::test::check("WM_DPICHANGED applies suggested rectangle",
                     actual.left == suggested.left
                     && actual.top == suggested.top
                     && actual.right == suggested.right
                     && actual.bottom == suggested.bottom);
    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
