#include "native_desktop/d3d12_presenter.h"
#include "ftd/interop_particle_record.h"
#include "ftd/test_telemetry.h"

// Off-screen: a message-only window is enough to build a swapchain-free
// D3D12 device via initialize()'s HWND-taking path. Simpler: construct a
// tiny hidden window here rather than stub initialize() into two halves --
// keeps this test exercising the real code path new callers will use.
#include <windows.h>

int main() {
    ftd::test::init("test_d3d12_shared_buffer");

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    ftd::native_desktop::D3D12Presenter presenter;
    presenter.initialize(hwnd, 64, 64);

    HANDLE handle = presenter.create_shared_particle_buffer(1000);
    ftd::test::check("shared buffer handle is non-null", handle != nullptr);
    ftd::test::check("shared_particle_buffer_bytes reports 1000 records",
                      presenter.shared_particle_buffer_bytes() ==
                          1000ull * sizeof(ftd::InteropParticleRecord));
    if (handle) CloseHandle(handle);

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
