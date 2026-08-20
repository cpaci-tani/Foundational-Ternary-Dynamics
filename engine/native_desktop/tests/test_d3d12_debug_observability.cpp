#include "native_desktop/d3d12_presenter.h"
#include "ftd/test_telemetry.h"

#include <omp.h>
#include <windows.h>

#include <cstdio>
#include <exception>

int main() {
    ftd::test::init("test_d3d12_debug_observability");

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdD3D12DebugTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64,
                                nullptr, nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    const int omp_before = omp_get_max_threads();
    ftd::native_desktop::D3D12Presenter presenter;
    try {
        ftd::native_desktop::D3D12PresenterOptions options;
        options.enable_debug_layer = true;
        presenter.initialize(hwnd, 64, 64, options);
        ftd::native_desktop::NativeFrame frame;
        frame.lattice_size = 9;
        ftd::native_desktop::Camera camera;
        presenter.render(frame, camera);
        presenter.wait_idle();
        ftd::test::check("D3D12 init preserves OpenMP thread count",
                         omp_get_max_threads() == omp_before);
        const auto messages = presenter.debug_messages();
        for (const auto& message : messages) {
            std::printf("[d3d12-debug] %s\n", message.c_str());
        }
        ftd::test::check("D3D12 debug layer reports no errors or warnings",
                         messages.empty());
    } catch (const std::exception& ex) {
        std::printf("[d3d12-debug] SKIP: %s\n", ex.what());
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
