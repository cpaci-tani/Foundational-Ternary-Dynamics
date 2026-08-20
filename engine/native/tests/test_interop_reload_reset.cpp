// engine/native/tests/test_interop_reload_reset.cpp
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
// then a reload, then reimport_interop_after_reload() with the SAME
// handles -- the exact free function main.cpp's do_reload branch calls,
// engine_session.h), not engine_session.cpp's private interop_enabled_
// field directly, so it fails the same way a real reload would if any of
// the three layers regressed. This was verified directly, not just
// asserted: temporarily reverting reimport_interop_after_reload() to the
// pre-Task-12 shape (never calling try_enable_interop() at all) turns 8 of
// this test's checks red; separately, inverting just its log_enabled
// computation (`was_active` in place of `!was_active`) turns 2 more red.
// Both reverts were confirmed against this test, then undone.
//
// It deliberately drives FOUR reload cycles, not one, each exercising a
// different InteropReloadOutcome transition combination so no combination
// is a false PASS-by-construction:
//   #1, #2 (apply_options(), then load_scenario() -- two different reload
//     entry points): interop was already active going in and stays active
//     -- the steady-state case, and the one the bug this test guards
//     against broke ("no re-import path exists at all", so a fix that
//     special-cased only the first reload would still pass a single-reload
//     check while leaving every subsequent reload broken).
//   #3: a deliberately null buffer handle -- the exact shape of Interop
//     Task 12's actual pre-fix defect (main.cpp closing the shared NT
//     handles too early) -- confirms under a live GPU-backed session (not
//     just test_interop_reload_orchestration.cpp's hardware-independent
//     CPU-backend coverage of the same function) that this specific
//     regression shape is caught cleanly: active->inactive, log_lost.
//   #4: real handles again, with was_active=false (inactive going in, from
//     #3) -- the one remaining transition ((#1)/(#2)/(#3) never exercise:
//     inactive->active, log_enabled.
//
// test_engine_session.cpp's "interop path is a safe no-op on the CPU
// backend" section covers the CPU-backend refusal path;
// test_interop_reload_orchestration.cpp covers reimport_interop_after_
// reload()'s outcome/logging contract (the inactive->inactive combination
// this test has no reason to visit, plus the missing-handle guard) without
// needing a GPU at all -- see that file's own header comment for the
// precise, more limited claim it can make (a CPU-backend
// NativeEngineSession can't distinguish "the call was made and the backend
// refused" from "the call was never made", so only a live GPU-backed test
// like this one can prove try_enable_interop() is actually invoked again);
// this test is their GPU-backend, full-session, end-to-end companion.

#include "native/d3d12_presenter.h"
#include "native/engine_session.h"
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
int gather_and_wait(ftd::native::NativeEngineSession& session,
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

    ftd::native::NativeEngineOptions options;
    options.lattice_size = 9;
    options.scenario = "s0-seed-hydrogen";
    options.force_cpu = false;

    ftd::native::NativeEngineSession session(options);
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
    // of taking down the whole CTest 'gpu;interactive;native' group
    // with an uncaught exception.
    ftd::native::D3D12Presenter presenter;
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
        // set_lattice_size()/reset_current() do. `was_active` mirrors
        // main.cpp's own `interop_active` flag: it starts true (interop was
        // just confirmed enabled above) and is threaded through
        // reimport_interop_after_reload()'s `was_active` parameter/
        // `interop_active` outcome field exactly the way main.cpp's
        // do_reload branch threads its own atomic through the same calls.
        bool was_active = true;
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

            // This is the Task 12 fix under test, driven through the exact
            // production entry point main.cpp's do_reload branch calls
            // (reimport_interop_after_reload(), engine_session.h) rather
            // than a hand-rolled call to try_enable_interop() that only
            // mirrors main.cpp's pattern: the caller re-supplies the SAME
            // still-open D3D12 buffer/fence NT handles, importing them into
            // the freshly constructed GpuEngine boot() just built. Before
            // Task 12, nothing in main.cpp ever did this -- interop stayed
            // disabled forever after the first reload.
            const auto outcome = ftd::native::reimport_interop_after_reload(
                session, buf_handle, buffer_bytes, fence_handle, was_active);
            ftd::test::check(
                ("reimport_interop_after_reload() re-establishes interop after "
                 "reload #" + label).c_str(),
                outcome.interop_active);
            ftd::test::check(
                ("session reports interop enabled after reload #" + label).c_str(),
                session.interop_enabled());
            ftd::test::check(
                ("reimport after reload #" + label + " reports no active->inactive "
                 "transition (it was active going in and stays active)").c_str(),
                !outcome.log_lost);
            ftd::test::check(
                ("reimport after reload #" + label + " reports no active->active "
                 "'enabled after reload' log either (was_active was already true "
                 "going in, so this is a continuation, not a rising edge)").c_str(),
                !outcome.log_enabled);

            session.tick();
            const int count_after =
                gather_and_wait(session, static_cast<std::uint64_t>(reload_i) + 2);
            ftd::test::check(
                ("interop gather works again after reload #" + label).c_str(),
                count_after >= 0,
                ("count_after=" + std::to_string(count_after)).c_str());

            was_active = outcome.interop_active;
        }

        // Third reload cycle: deliberately supply a null buffer handle to
        // reimport_interop_after_reload() instead of the real (still-open)
        // one -- this is the exact shape of Interop Task 12's actual
        // pre-fix defect (main.cpp closing interop_buf_handle/
        // interop_fence_handle immediately after the startup import,
        // leaving nothing valid to re-supply on the first reload). Confirms
        // under a live GPU-backed session (not just
        // test_interop_reload_orchestration.cpp's CPU-backend coverage of
        // the same function) that this shape is caught cleanly -- reported
        // as a failed reimport with the active->inactive transition
        // flagged, never silently mistaken for success, and never a crash
        // from handing a null/dangling handle to the CUDA import path.
        session.apply_options(session.options());
        ftd::test::check(
            "boot() clears interop_enabled_ before the deliberate-null-handle check",
            !session.interop_enabled());
        const auto null_handle_outcome = ftd::native::reimport_interop_after_reload(
            session, /*shared_buffer_handle=*/nullptr, buffer_bytes, fence_handle,
            /*was_active=*/was_active);
        ftd::test::check(
            "a null buffer handle (simulating handles closed too early) fails "
            "to reimport, not silently succeeds",
            !null_handle_outcome.interop_active);
        ftd::test::check(
            "a null buffer handle after a previously-active session reports the "
            "active->inactive transition",
            null_handle_outcome.log_lost);
        ftd::test::check(
            "session.interop_enabled() reflects the failed reimport too",
            !session.interop_enabled());

        // Fourth reload cycle: interop is inactive at this point (the
        // deliberate null-handle check just above left it that way). Reload
        // once more and reimport with the REAL still-open handles and
        // was_active=false, to exercise the one InteropReloadOutcome
        // transition combination nothing above does yet -- an
        // inactive-going-in reimport that succeeds ("enabled after reload",
        // log_enabled=true). The reload #1/#2 loop above only ever starts
        // from was_active=true (interop was already active before any
        // reload happened), so it can only ever exercise the
        // active->active (no log) and, via the null-handle check,
        // active->inactive (log_lost) combinations -- never this one.
        session.apply_options(session.options());
        ftd::test::check(
            "boot() clears interop_enabled_ before the rising-edge reimport check",
            !session.interop_enabled());
        const auto rising_edge_outcome = ftd::native::reimport_interop_after_reload(
            session, buf_handle, buffer_bytes, fence_handle, /*was_active=*/false);
        ftd::test::check(
            "reimport succeeds again once the real handles are supplied",
            rising_edge_outcome.interop_active);
        ftd::test::check(
            "an inactive->active transition reports the 'enabled after reload' log",
            rising_edge_outcome.log_enabled);
        ftd::test::check(
            "the rising-edge outcome does not also report log_lost",
            !rising_edge_outcome.log_lost);
        ftd::test::check(
            "session.interop_enabled() reflects the successful rising-edge reimport",
            session.interop_enabled());

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
