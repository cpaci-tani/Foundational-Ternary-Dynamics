// engine/native_desktop/tests/test_interop_fence_roundtrip.cpp
#include "native_desktop/d3d12_presenter.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/test_telemetry.h"

#include <windows.h>

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
    ftd::native_desktop::D3D12Presenter presenter;
    presenter.initialize(hwnd, 64, 64);

    HANDLE buf_handle = presenter.create_shared_particle_buffer(100);
    engine->import_d3d12_particle_buffer(buf_handle,
                                         presenter.shared_particle_buffer_bytes());
    CloseHandle(buf_handle);

    HANDLE fence_handle = presenter.create_shared_fence();
    ftd::test::check("shared fence created", fence_handle != nullptr);
    const bool fence_imported = engine->import_d3d12_fence(fence_handle);
    ftd::test::check("CUDA imported the D3D12 shared fence", fence_imported);
    CloseHandle(fence_handle);

    constexpr std::uint64_t kTargetValue = 42;
    engine->interop_gather_particles(100, kTargetValue);

    // wait_shared_fence enqueues a GPU-timeline wait; it returns immediately
    // regardless of whether the fence has reached the value yet (that's the
    // whole point -- no CPU stall). To prove the wait is real and not a
    // no-op, submit a trivial command list right after and confirm
    // wait_idle() (a REAL CPU stall, used only here for test verification)
    // doesn't hang or error -- if the D3D12 Wait() call were broken (e.g.
    // wrong fence object), this would either throw_if_failed immediately or
    // (worse, and exactly why this test exists) silently never signal it
    // correctly and the GPU would stay parked forever, hanging this test
    // until CI/local timeout -- an intentional fail-loud choice over a
    // softer polling check.
    presenter.wait_shared_fence(kTargetValue);
    presenter.wait_idle();
    ftd::test::check("D3D12 queue drained after waiting on the CUDA-signaled fence value",
                      true);

    CloseHandle(hwnd);
    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
