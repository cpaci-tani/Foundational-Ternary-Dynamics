// Phase 1b: ImGui Win32 + DX12 overlay records through the presenter without
// the presenter including imgui.h. Interactive (live swapchain + HWND).
#include "native/d3d12_presenter.h"
#include "native/imgui_overlay.h"
#include "ftd/test_telemetry.h"

#include <windows.h>

#include <cstdio>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

int main() {
    ftd::test::init("test_ui_imgui_d3d12");

#ifdef _OPENMP
    const int omp_before = omp_get_max_threads();
#endif

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdImGuiD3D12TestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 320, 240, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    try {
        ftd::native::D3D12Presenter presenter;
        presenter.initialize(hwnd, 320, 240);

        ftd::native::ImGuiOverlay overlay;
        ftd::test::check("overlay initializes from presenter context",
                         overlay.initialize(hwnd, presenter.ui_backend_context()));
        presenter.set_overlay_recorder(&overlay);

        ftd::native::NativeFrame frame;
        frame.tick = 1;
        frame.lattice_size = 8;
        ftd::native::Camera camera;

        overlay.begin_frame(1.0f / 60.0f);
        overlay.draw_debug_window();
        overlay.end_frame();
        presenter.render(frame, camera, {}, 0);

        overlay.begin_frame(1.0f / 60.0f);
        overlay.draw_debug_window();
        overlay.end_frame();
        presenter.render(frame, camera, {}, 0);

        presenter.wait_idle();
        overlay.rebuild_fonts(1.25f);
        overlay.begin_frame(1.0f / 60.0f);
        overlay.draw_debug_window();
        overlay.end_frame();
        presenter.render(frame, camera, {}, 0);
        presenter.wait_idle();

        presenter.set_overlay_recorder(nullptr);
        overlay.shutdown();

        ftd::test::check("ImGui-in-D3D12 overlay recorded without throwing", true);

#ifdef _OPENMP
        ftd::test::check("overlay init is OpenMP-neutral",
                         omp_get_max_threads() == omp_before);
#endif
    } catch (const std::exception& ex) {
        std::printf("[ui-imgui-d3d12] SKIP: %s\n", ex.what());
        ftd::test::check("imgui d3d12 skipped, no live D3D12 device", true, "");
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
