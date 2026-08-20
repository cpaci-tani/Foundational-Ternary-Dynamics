#include "native/d3d12_presenter.h"
#include "ftd/render_bridge.h"
#include "ftd/gpu_engine.h"
#include "ftd/test_telemetry.h"

#include <windows.h>
#include <cstdio>
#include <stdexcept>
#include <string>

int main() {
    ftd::test::init("test_interop_gather");

    ftd::RenderBridge rb(9);
    rb.set_interactive_gpu_mode(true);
    if (rb.backend_kind() != ftd::Backend::Kind::Gpu) {
        std::printf("[interop-gather] SKIP: no GPU backend in this build\n");
        ftd::test::check("interop gather skipped on CPU-only build", true, "");
        return ftd::test::finalize();
    }
    ftd::gpu::GpuEngine* engine = rb.gpu_engine_ptr();

    // This test's assertion is about the gather kernel's selection/count
    // logic, not the engine's stochastic Boltzmann evaporation (genesis
    // toggle, kernels_stencil_single.cu evaporation_kernel comment / BH-F5):
    // a manifested particle injected with zero flux has local_energy == 0,
    // so evap_prob == 1 and survival is a per-voxel coin flip gated only by
    // K_EVAP_RATE. With genesis on (the default), that stochastic lifecycle
    // pass runs on the very first tick() below and can legitimately
    // de-manifest one of the three injected particles before the interop
    // gather ever sees it -- not a gather-kernel bug, just an out-of-scope
    // source of flakiness for this count check. Disabling genesis (and
    // evaporation, off by default but disabled explicitly for clarity) keeps
    // the manifested count deterministic without touching interop_gather
    // itself, which never reads toggles at all.
    rb.toggles.genesis = false;
    rb.toggles.evaporation = false;

    rb.inject_particle(2, 2, 2, +1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(6, 6, 6, -1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(4, 2, 6, +1, ftd::Vec3{0.0, 0.0, 0.0});
    rb.tick();  // sync toggles/state to the GPU engine

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropGatherTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    // D3D12Presenter::initialize() throw_if_failed()s on any DXGI/D3D12
    // call that fails (e.g. no usable hardware adapter, or swap-chain
    // creation failing on a headless/RDP/CI session with no live display).
    // Same try/catch SKIP pattern as this test's direct precedents,
    // test_cuda_import_shared_buffer.cpp and test_d3d12_shared_buffer.cpp,
    // so a machine without a usable D3D12 adapter/display skips cleanly
    // instead of taking down the whole CTest 'gpu;interactive;native'
    // group with an uncaught exception.
    ftd::native::D3D12Presenter presenter;
    HANDLE handle = nullptr;
    try {
        presenter.initialize(hwnd, 64, 64);
        handle = presenter.create_shared_particle_buffer(1000);
        ftd::test::check("shared handle created", handle != nullptr);
        if (!handle) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        const bool imported = engine->import_d3d12_particle_buffer(
            handle, presenter.shared_particle_buffer_bytes());
        // The NT handle can be closed immediately after import per the CUDA
        // Runtime API contract for cudaExternalMemoryHandleDesc (also
        // exercised in test_cuda_import_shared_buffer.cpp). HWND is a
        // USER-object handle, not a kernel-object handle -- only
        // DestroyWindow(), never CloseHandle(), tears it down; DestroyWindow
        // is called once, unconditionally, after this try block.
        CloseHandle(handle);
        handle = nullptr;
        ftd::test::check("import succeeded", imported);

        // fence_value is unused here -- this test never imports a fence via
        // import_d3d12_fence(), so interop_gather_particles()'s fence-signal
        // branch is a no-op regardless of what value is passed (Task 7).
        const bool started = engine->interop_gather_particles(1000, 0);
        ftd::test::check("interop gather started", started);

        bool ready = false;
        for (int i = 0; i < 5000 && !ready; ++i) {
            ready = engine->interop_gather_ready();
            if (!ready) Sleep(0);
        }
        ftd::test::check("interop gather completed", ready);

        const std::uint32_t count = engine->interop_particle_count();
        ftd::test::check("interop particle count matches the 3 injected particles",
                          count == 3,
                          ("count=" + std::to_string(count)).c_str());
    } catch (const std::exception& ex) {
        std::printf("[interop-gather] SKIP: D3D12 setup failed: %s\n", ex.what());
        ftd::test::check("interop gather skipped, no live D3D12 device", true, "");
        if (handle) CloseHandle(handle);
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
