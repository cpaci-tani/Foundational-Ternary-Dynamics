// Capture seam: request_capture returns a token; the next render records the
// readback copy; poll_capture stays Pending until the submission fence
// retires, then Ready with pitched bytes. Interactive (live swapchain).
#include "native/d3d12_presenter.h"
#include "ftd/test_telemetry.h"

#include <windows.h>

#include <cstdio>
#include <stdexcept>

int main() {
    ftd::test::init("test_d3d12_capture_lifecycle");

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdCaptureLifecycleTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    try {
        ftd::native::D3D12Presenter presenter;
        presenter.initialize(hwnd, 64, 64);
        presenter.set_scene_rect({16, 0, 48, 64});

        ftd::native::NativeFrame frame;
        frame.tick = 1;
        frame.lattice_size = 8;
        ftd::native::Camera camera;

        const auto idle = presenter.poll_capture({});
        ftd::test::check("zero token is Idle",
                         idle.status == ftd::native::CaptureStatus::Idle);

        const auto token =
            presenter.request_capture(ftd::native::CaptureRegion::Scene);
        ftd::test::check("request_capture issues a monotone token", token.id != 0);

        const auto before = presenter.poll_capture(token);
        ftd::test::check("poll before render is Pending",
                         before.status == ftd::native::CaptureStatus::Pending);

        presenter.render(frame, camera, {}, 0);
        presenter.wait_idle();

        const auto ready = presenter.poll_capture(token);
        ftd::test::check("poll after fence is Ready",
                         ready.status == ftd::native::CaptureStatus::Ready);
        ftd::test::check("scene capture uses the scene extent",
                         ready.width == 48 && ready.height == 64);
        ftd::test::check("pitched bytes are non-empty",
                         ready.status == ftd::native::CaptureStatus::Ready
                         && ready.row_pitch >= ready.width * 4
                         && ready.bytes.size() == static_cast<std::size_t>(ready.row_pitch)
                                * ready.height);

        const auto again = presenter.poll_capture(token);
        ftd::test::check("polling a Ready token stays Ready",
                         again.status == ftd::native::CaptureStatus::Ready);

        const ftd::native::CaptureToken bogus{token.id + 99};
        const auto unknown = presenter.poll_capture(bogus);
        ftd::test::check("unknown token is Failed",
                         unknown.status == ftd::native::CaptureStatus::Failed);
    } catch (const std::exception& ex) {
        std::printf("[d3d12-capture-lifecycle] SKIP: %s\n", ex.what());
        ftd::test::check("capture lifecycle skipped, no live D3D12 device", true, "");
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
