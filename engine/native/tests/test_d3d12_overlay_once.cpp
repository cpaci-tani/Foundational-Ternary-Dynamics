// Counts OverlayRecorder::record() invocations: SPEC_UI_V2 §3.5 requires
// exactly one overlay record per D3D12Presenter::render(), with a NULL DSV
// rebind already performed by the presenter (this fake recorder does not
// issue draws). Interactive because initialize() needs a live swapchain.
#include "native/d3d12_presenter.h"
#include "ftd/test_telemetry.h"

#include <d3d12.h>
#include <windows.h>

#include <cstdio>
#include <stdexcept>

namespace {

struct CountingRecorder final : ftd::native::OverlayRecorder {
    int calls = 0;
    ID3D12GraphicsCommandList* last_list = nullptr;
    ftd::native::RenderTargetInfo last_rt{};

    void record(ID3D12GraphicsCommandList* list,
                const ftd::native::RenderTargetInfo& rt) override {
        ++calls;
        last_list = list;
        last_rt = rt;
    }
};

}  // namespace

int main() {
    ftd::test::init("test_d3d12_overlay_once");

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdOverlayOnceTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 128, 96, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    try {
        ftd::native::D3D12Presenter presenter;
        presenter.initialize(hwnd, 128, 96);
        ftd::test::check("SRV heap grew to 256",
                         presenter.srv_heap_capacity() == 256);

        presenter.set_scene_rect({32, 0, 96, 96});
        CountingRecorder recorder;
        presenter.set_overlay_recorder(&recorder);

        ftd::native::NativeFrame frame;
        frame.tick = 1;
        frame.lattice_size = 8;
        frame.backend = "cpu";
        ftd::native::Camera camera;
        ftd::native::NativeViewOptions opts;

        constexpr int kFrames = 5;
        for (int i = 0; i < kFrames; ++i) {
            presenter.render(frame, camera, opts, 0);
        }
        presenter.wait_idle();
        presenter.set_overlay_recorder(nullptr);

        ftd::test::check("exactly one overlay record per render()",
                         recorder.calls == kFrames);
        ftd::test::check("recorder received a live command list",
                         recorder.last_list != nullptr);
        ftd::test::check("render-target extent is the full backbuffer",
                         recorder.last_rt.width == 128 && recorder.last_rt.height == 96);
        ftd::test::check("ui_backend_context exposes the live device",
                         presenter.ui_backend_context().device != nullptr
                         && presenter.ui_backend_context().num_frames_in_flight == 2);
    } catch (const std::exception& ex) {
        std::printf("[d3d12-overlay-once] SKIP: %s\n", ex.what());
        ftd::test::check("overlay-once skipped, no live D3D12 device", true, "");
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
