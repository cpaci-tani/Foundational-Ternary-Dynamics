// engine/native_desktop/tests/test_interop_reload_reset.cpp
//
// Regression coverage for two related Task 9/Task 12 findings:
//
//   1. (Task 9 round-2 review, fixed) NativeEngineSession::boot() must clear
//      interop_enabled_ on every reload (apply_options()/load_scenario()/
//      set_lattice_size()/reset_current() all funnel through boot()) -- a
//      freshly-constructed GpuEngine has never had anything imported into
//      it, so interop_enabled() must report false immediately after a
//      reload, or request_interop_gather()/poll_interop_particle_count()
//      keep being invoked against an engine that can never satisfy them.
//
//   2. (Task 9 review, found live, fixed by Task 12) That clearing left a
//      gap: nothing ever re-imported the shared D3D12 buffer/fence into the
//      freshly-constructed GpuEngine afterward, so main.cpp's real reload
//      path (the 'R' key, Load scenario, or a lattice-size change)
//      permanently fell back to the CPU particle-capture path after the
//      very first reload of the process. Task 12's fix: main.cpp now keeps
//      the original D3D12Presenter-issued NT handles open for the process
//      lifetime (D3D12Presenter itself, unlike NativeEngineSession's
//      internal bridge_/GpuEngine, is never destroyed by a reload) and
//      re-supplies them to NativeEngineSession::try_enable_interop() again
//      once boot() has finished rebuilding bridge_/GpuEngine.
//
// This test exercises that exact caller contract through the same public
// NativeEngineSession API main.cpp itself drives (try_enable_interop(),
// then a reload, then try_enable_interop() again with the SAME handles),
// not engine_session.cpp's private interop_enabled_ field directly, so it
// fails the same way a real reload would if either half regressed. It
// deliberately drives TWO reload cycles (via two different reload entry
// points) rather than one: the bug this guards against was "no re-import
// path exists at all", so a fix that special-cased only the first reload
// would still pass a single-reload check while leaving every subsequent
// reload broken.
//
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

namespace {

// Drives one request_interop_gather()/poll_interop_particle_count() cycle
// to completion, mirroring test_interop_gather.cpp's busy-wait idiom
// (engine->interop_gather_ready()) at the NativeEngineSession level.
// Returns the polled particle count once ready, or -1 if it never became
// ready within the retry budget.
int gather_and_wait(ftd::native_desktop::NativeEngineSession& session,
                    std::uint64_t fence_value) {
    session.request_interop_gather(fence_value);
    int count = -1;
    for (int i = 0; i < 5000 && count == -1; ++i) {
        count = session.poll_interop_particle_count();
        if (count == -1) Sleep(0);
    }
    return count;
}

}  // namespace

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
        const std::uint64_t buffer_bytes = presenter.shared_particle_buffer_bytes();

        // Unlike the pre-Task-12 version of this test, buf_handle/
        // fence_handle are kept open for this test's entire lifetime
        // instead of being closed immediately after the first import: that
        // mirrors exactly what main.cpp now does (see its do_reload branch)
        // so that every later reload can present the SAME still-valid NT
        // handles to try_enable_interop() again. CUDA never takes ownership
        // of these handles (GpuEngine::import_d3d12_particle_buffer's own
        // doc comment: "The handle is NOT closed by this call") so reusing
        // them across multiple sequential imports is within contract; both
        // are closed exactly once, at the very end of this test, mirroring
        // main.cpp closing them once at process shutdown.
        const bool enabled =
            session.try_enable_interop(buf_handle, buffer_bytes, fence_handle);
        ftd::test::check("interop enabled against a live GPU session", enabled);
        if (!enabled) {
            CloseHandle(buf_handle);
            CloseHandle(fence_handle);
            DestroyWindow(hwnd);
            return ftd::test::finalize();
        }
        ftd::test::check("session reports interop enabled before reload",
                         session.interop_enabled());

        session.tick();
        const int count_before = gather_and_wait(session, 1);
        ftd::test::check("interop gather works before any reload",
                         count_before >= 0,
                         ("count_before=" + std::to_string(count_before)).c_str());

        // Two reload cycles via two different NativeEngineSession reload
        // entry points -- apply_options() (the 'R'-key/Reset-button
        // equivalent) then load_scenario() (the Load-scenario-button
        // equivalent) -- both of which funnel through boot() exactly like
        // set_lattice_size()/reset_current() do.
        for (int reload_i = 0; reload_i < 2; ++reload_i) {
            const std::string label = std::to_string(reload_i + 1);
            if (reload_i == 0) {
                session.apply_options(session.options());
            } else {
                session.load_scenario("s0-seed-ee-annihilation");
            }

            // boot() (invoked by every reload entry point) always clears
            // interop_enabled_ -- it tears down bridge_/GpuEngine and
            // constructs a fresh one, and nothing has imported into that
            // fresh GpuEngine yet. This half of the regression coverage is
            // unchanged from before Task 12: reload must still start from a
            // clean slate, never carry a stale interop_enabled_ == true
            // forward against a GpuEngine nothing has imported into.
            ftd::test::check(
                ("boot() clears interop_enabled_ immediately after reload #" + label)
                    .c_str(),
                !session.interop_enabled());
            ftd::test::check(
                ("poll_interop_particle_count reports not-ready (-1) immediately "
                 "after reload #" + label + ", before re-import")
                    .c_str(),
                session.poll_interop_particle_count() == -1);

            // This is the Task 12 fix under test: the caller re-supplies the
            // SAME still-open D3D12 buffer/fence NT handles to
            // try_enable_interop(), importing them into the freshly
            // constructed GpuEngine boot() just built. Before Task 12,
            // nothing in main.cpp ever did this -- interop stayed disabled
            // forever after the first reload. This call is exactly what
            // main.cpp's do_reload branch now performs.
            const bool reimported =
                session.try_enable_interop(buf_handle, buffer_bytes, fence_handle);
            ftd::test::check(
                ("try_enable_interop() re-establishes interop after reload #" + label)
                    .c_str(),
                reimported);
            ftd::test::check(
                ("session reports interop enabled after reload #" + label).c_str(),
                session.interop_enabled());

            session.tick();
            const int count_after =
                gather_and_wait(session, static_cast<std::uint64_t>(reload_i) + 2);
            ftd::test::check(
                ("interop gather works again after reload #" + label).c_str(),
                count_after >= 0,
                ("count_after=" + std::to_string(count_after)).c_str());
        }

        CloseHandle(buf_handle);
        CloseHandle(fence_handle);
        buf_handle = nullptr;
        fence_handle = nullptr;
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
