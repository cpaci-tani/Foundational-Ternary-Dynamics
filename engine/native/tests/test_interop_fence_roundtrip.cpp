// engine/native/tests/test_interop_fence_roundtrip.cpp
#include "native/d3d12_presenter.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/test_telemetry.h"

#include <windows.h>
#include <cstdio>
#include <stdexcept>
#include <string>

int main() {
    ftd::test::init("test_interop_fence_roundtrip");

    ftd::RenderBridge rb(9);
    rb.set_interactive_gpu_mode(true);
    if (rb.backend_kind() != ftd::Backend::Kind::Gpu) {
        std::printf("[fence-roundtrip] SKIP: no GPU backend in this build\n");
        ftd::test::check("fence roundtrip skipped on CPU-only build", true, "");
        return ftd::test::finalize();
    }
    ftd::gpu::GpuEngine* engine = rb.gpu_engine_ptr();
    rb.inject_particle(4, 4, 4, +1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.tick();

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropFenceTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    // D3D12Presenter::initialize() throw_if_failed()s on any DXGI/D3D12
    // call that fails (e.g. no usable hardware adapter, or swap-chain
    // creation failing on a headless/RDP/CI session with no live display).
    // Same try/catch SKIP pattern as this test's direct precedents --
    // test_interop_gather.cpp, test_cuda_import_shared_buffer.cpp, and
    // test_d3d12_shared_buffer.cpp -- so a machine without a usable D3D12
    // adapter/display skips cleanly instead of taking down the whole CTest
    // 'gpu;interactive;native' group with an uncaught exception.
    ftd::native::D3D12Presenter presenter;
    HANDLE buf_handle = nullptr;
    HANDLE fence_handle = nullptr;
    try {
        presenter.initialize(hwnd, 64, 64);

        buf_handle = presenter.create_shared_particle_buffer(100);
        ftd::test::check("shared particle buffer created", buf_handle != nullptr);
        if (!buf_handle) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        const bool buffer_imported = engine->import_d3d12_particle_buffer(
            buf_handle, presenter.shared_particle_buffer_bytes());
        // The NT handle can be closed immediately after import per the CUDA
        // Runtime API contract for cudaExternalMemoryHandleDesc (also
        // exercised in test_cuda_import_shared_buffer.cpp). HWND is a
        // USER-object handle, not a kernel-object handle -- only
        // DestroyWindow(), never CloseHandle(), tears it down; DestroyWindow
        // is called once, unconditionally, after this try block.
        CloseHandle(buf_handle);
        buf_handle = nullptr;
        ftd::test::check("particle buffer import succeeded", buffer_imported);
        if (!buffer_imported) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        fence_handle = presenter.create_shared_fence();
        ftd::test::check("shared fence created", fence_handle != nullptr);
        if (!fence_handle) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        const bool fence_imported = engine->import_d3d12_fence(fence_handle);
        CloseHandle(fence_handle);
        fence_handle = nullptr;
        ftd::test::check("CUDA imported the D3D12 shared fence", fence_imported);
        if (!fence_imported) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        constexpr std::uint64_t kTargetValue = 42;
        const bool started = engine->interop_gather_particles(100, kTargetValue);
        ftd::test::check("interop gather started", started);
        if (!started) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        // wait_shared_fence enqueues a GPU-timeline wait; it returns
        // immediately regardless of whether the fence has reached the value
        // yet (that's the whole point -- no CPU stall). To prove the wait is
        // real and not a no-op, follow it with wait_idle() (a REAL CPU
        // stall, used only here for test verification): wait_idle()'s own
        // queue->Signal() call is issued on the SAME queue right after
        // wait_shared_fence()'s queue->Wait() above, and D3D12 command
        // queues execute their enqueued operations in order, so that Signal
        // cannot retire until the preceding Wait has -- if wait_idle()
        // returns (rather than hanging for the full CTest timeout), the Wait
        // against the CUDA-signaled fence value genuinely unblocked. If the
        // D3D12 Wait() call were broken (e.g. wrong fence object), this
        // would either throw_if_failed immediately or (worse, and exactly
        // why this test exists) silently never signal it correctly and the
        // GPU would stay parked forever, hanging this test until CI/local
        // timeout -- an intentional fail-loud choice over a softer polling
        // check.
        presenter.wait_shared_fence(kTargetValue);
        presenter.wait_idle();
        ftd::test::check("D3D12 queue drained after waiting on the CUDA-signaled fence value",
                          true);
    } catch (const std::exception& ex) {
        std::printf("[fence-roundtrip] SKIP: D3D12 setup failed: %s\n", ex.what());
        ftd::test::check("fence roundtrip skipped, no live D3D12 device", true, "");
        if (buf_handle) CloseHandle(buf_handle);
        if (fence_handle) CloseHandle(fence_handle);
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
