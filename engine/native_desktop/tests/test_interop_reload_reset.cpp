// engine/native_desktop/tests/test_interop_reload_reset.cpp
//
// Regression coverage for the round-2 code-review finding that
// NativeEngineSession::boot() never reset interop_enabled_ after a reload:
// with a live D3D12/CUDA interop import already in place, ANY reload (the
// 'R' key, scenario Load, or a lattice-size change in the real app --
// engine_session.cpp's apply_options()/load_scenario()/set_lattice_size()/
// reset_current() all funnel through boot()) used to keep reporting
// interop_enabled() == true against a freshly-constructed GpuEngine that had
// never been re-imported into, so request_interop_gather()/
// poll_interop_particle_count() kept firing against a dead interop path
// forever with no diagnostic.
//
// This exercises the fix through the same public NativeEngineSession API
// main.cpp itself drives (try_enable_interop() then a reload call), not
// engine_session.cpp's private interop_enabled_ field directly, so it fails
// the same way a real 'R'-key reload would if the reset regressed.
// test_engine_session.cpp's "interop path is a safe no-op on the CPU
// backend" section covers the CPU-backend refusal path; this test is its
// GPU-backend companion.

#include "native_desktop/d3d12_presenter.h"
#include "native_desktop/engine_session.h"
#include "ftd/test_telemetry.h"

#include <windows.h>
#include <cstdio>
#include <stdexcept>
#include <string>

int main() {
    ftd::test::init("test_interop_reload_reset");

    ftd::native_desktop::NativeEngineOptions options;
    options.lattice_size = 9;
    options.scenario = "s0-seed-hydrogen";
    options.force_cpu = false;

    ftd::native_desktop::NativeEngineSession session(options);
    if (std::string(session.backend_name()) != "cuda") {
        std::printf("[interop-reload-reset] SKIP: no GPU backend in this build\n");
        ftd::test::check("interop reload reset skipped on CPU-only build", true, "");
        return ftd::test::finalize();
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdInteropReloadResetTestWindow";
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 64, 64, nullptr,
                                nullptr, wc.hInstance, nullptr);
    ftd::test::check("test window created", hwnd != nullptr);
    if (!hwnd) return ftd::test::finalize();

    // D3D12Presenter::initialize() throw_if_failed()s on any DXGI/D3D12 call
    // that fails (e.g. no usable hardware adapter, or swap-chain creation
    // failing on a headless/RDP/CI session with no live display). Same
    // try/catch SKIP pattern as this test's direct precedents --
    // test_interop_gather.cpp and test_interop_fence_roundtrip.cpp -- so a
    // machine without a usable D3D12 adapter/display skips cleanly instead
    // of taking down the whole CTest 'gpu;interactive;native_desktop' group
    // with an uncaught exception.
    ftd::native_desktop::D3D12Presenter presenter;
    HANDLE buf_handle = nullptr;
    HANDLE fence_handle = nullptr;
    try {
        presenter.initialize(hwnd, 64, 64);
        buf_handle = presenter.create_shared_particle_buffer(1000);
        fence_handle = buf_handle ? presenter.create_shared_fence() : nullptr;
        ftd::test::check("shared buffer + fence created", buf_handle && fence_handle);
        if (!buf_handle || !fence_handle) {
            if (buf_handle) CloseHandle(buf_handle);
            if (fence_handle) CloseHandle(fence_handle);
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }

        const bool enabled = session.try_enable_interop(
            buf_handle, presenter.shared_particle_buffer_bytes(), fence_handle);
        // Same NT-handle lifetime contract as every other interop test: the
        // handle may be closed immediately once the import call returns,
        // success or failure (cudaExternalMemoryHandleDesc's contract, and
        // D3D12Presenter's fence handle likewise).
        CloseHandle(buf_handle);
        CloseHandle(fence_handle);
        buf_handle = nullptr;
        fence_handle = nullptr;
        ftd::test::check("interop enabled against a live GPU session", enabled);
        if (!enabled) {
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }
        ftd::test::check("session reports interop enabled before reload",
                         session.interop_enabled());

        // Any of NativeEngineSession's reload entry points (apply_options()
        // -- the 'R' key's equivalent --, load_scenario(), set_lattice_size(),
        // reset_current()) run boot() start to finish and must clear
        // interop_enabled_. Exercise load_scenario() here: it changes
        // nothing about the D3D12 side (no new buffer/fence, no
        // presenter call), isolating the assertion to exactly the
        // regression -- does boot() clear interop_enabled_ even when the
        // shared buffer/fence handles themselves are still perfectly valid.
        session.load_scenario("s0-seed-ee-annihilation");
        ftd::test::check(
            "reload clears interop_enabled_ (this is the fix for the "
            "'stale interop_enabled_ after reload' finding -- no re-import "
            "path exists yet, so staying disabled post-reload is the "
            "correct/expected behavior here, not a bug)",
            !session.interop_enabled());

        // The disabled state must also make the public polling/request API
        // fail closed instead of reaching back into the torn-down GpuEngine
        // import -- same -1/no-op contract poll_interop_particle_count()
        // documents for "interop never enabled" (test_engine_session.cpp
        // covers that contract on the CPU-backend-refusal path).
        ftd::test::check("poll_interop_particle_count reports not-ready (-1) after reload",
                         session.poll_interop_particle_count() == -1);
        session.request_interop_gather(1);  // must be a harmless no-op, not a crash
        ftd::test::check("poll after a post-reload no-op gather request still reports -1",
                         session.poll_interop_particle_count() == -1);
    } catch (const std::exception& ex) {
        std::printf("[interop-reload-reset] SKIP: D3D12 setup failed: %s\n", ex.what());
        ftd::test::check("interop reload reset skipped, no live D3D12 device", true, "");
        if (buf_handle) CloseHandle(buf_handle);
        if (fence_handle) CloseHandle(fence_handle);
        DestroyWindow(hwnd);
        return ftd::test::finalize();
    }

    DestroyWindow(hwnd);
    return ftd::test::finalize();
}
