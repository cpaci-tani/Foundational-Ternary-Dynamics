// engine/native/tests/test_interop_reload_orchestration.cpp
//
// Fast, hardware-independent regression coverage for the InteropReloadOutcome
// transition/logging contract of reimport_interop_after_reload()
// (engine_session.h) -- the exact free function main.cpp's do_reload branch
// calls to re-establish D3D12/CUDA interop after every reload (Interop
// Task 12, commit 93d03a3c). It runs against a CPU-backend
// NativeEngineSession, so it needs no GPU/display and is registered outside
// any FTD_ENABLE_CUDA gate in engine/CMakeLists.txt -- it always runs, in
// every build and on every machine, unlike every other interop-labeled test
// in this directory.
//
// IMPORTANT SCOPE LIMIT, verified directly rather than assumed: on the CPU
// backend, NativeEngineSession::try_enable_interop() returns false before
// touching either handle argument (engine_session.cpp: "if
// (bridge_->backend_kind() != Backend::Kind::Gpu) return false;"), which
// means "the call was made and the backend correctly refused" and "the call
// was never made at all" are OUTWARDLY IDENTICAL on this backend -- this
// test therefore cannot independently prove reimport_interop_after_reload()
// actually invokes try_enable_interop() when live-looking handles are
// supplied, nor can it exercise the reimported=true/log_enabled=true branch
// at all (reimported can never become true on the CPU backend). Confirmed
// by deliberately reverting reimport_interop_after_reload() to the
// pre-Task-12 shape (hardcoding reimported=false, never calling
// try_enable_interop()) and rerunning this test: it still passed, 0
// failures. Only test_interop_reload_reset.cpp's live GPU-backed session
// can (and does -- see that file's header comment for the confirmed
// red/green revert) prove the call itself happens and succeeds.
//
// What THIS test does verify, deterministically and CPU-only: given
// whatever boolean try_enable_interop() (or the missing-handle guard ahead
// of it) produces, reimport_interop_after_reload() maps
// (reimported, was_active) to (interop_active, log_enabled, log_lost)
// correctly for every combination reachable when reimported is false --
// which is the inactive->inactive and active->inactive combinations plus
// the missing-handle guard's own bookkeeping. (The remaining two
// combinations -- active->active and inactive->active, both requiring
// reimported=true -- are covered by test_interop_reload_reset.cpp's live
// GPU session instead.) This closes a real, separately-confirmed gap: a
// deliberate revert of just the log_enabled/log_lost computation (flipping
// `!was_active` to `was_active`) DOES turn checks in this file red.
//
// Together with the extraction itself (main.cpp's do_reload branch is now a
// thin, no-decision-logic-of-its-own caller of reimport_interop_after_
// reload(), instead of owning ~55 lines of inline guard/logging logic with
// zero ctest surface), this test and test_interop_reload_reset.cpp jointly
// close the spec-compliance gap raised against 93d03a3c/7173a311: main.cpp
// still calling this function at all, and main.cpp still keeping
// interop_buf_handle/interop_fence_handle open across a reload instead of
// closing them right after the startup import, remain the only residual
// facts outside ctest's reach (main.cpp is a WinMain GUI executable with no
// ctest harness of its own) -- covered the way this codebase's established
// precedent covers other main.cpp-only fixes (be7eef14, 1b80fb53): code
// review plus a manual multi-reload smoke pass on the built exe (93d03a3c's
// commit message).

#include "ftd/test_telemetry.h"
#include "native/engine_session.h"

#include <cstdint>
#include <string>

int main() {
    ftd::test::init("test_interop_reload_orchestration");

    ftd::native::NativeEngineOptions options;
    options.lattice_size = 9;
    options.scenario = "s0-seed-hydrogen";
    options.force_cpu = true;

    ftd::native::NativeEngineSession session(options);
    ftd::test::check("fixture session uses the CPU backend",
                     std::string(session.backend_name()) == "cpu");

    // On the CPU backend, try_enable_interop() returns false before ever
    // dereferencing either handle argument (see this file's header comment
    // above), so any non-null value is safe to pass here -- these do not
    // need to be real Windows HANDLEs, just non-null void* sentinels
    // distinguishable from the "missing/closed handle" nullptr case under
    // test below.
    void* const fake_buf = reinterpret_cast<void*>(static_cast<std::uintptr_t>(1));
    void* const fake_fence = reinterpret_cast<void*>(static_cast<std::uintptr_t>(2));
    constexpr std::uint64_t kFakeBytes = 4096;

    ftd::test::section("valid-looking handles, backend still refuses (CPU)");
    {
        const auto outcome = ftd::native::reimport_interop_after_reload(
            session, fake_buf, kFakeBytes, fake_fence, /*was_active=*/false);
        ftd::test::check("reimport fails on the CPU backend even with live-looking handles",
                         !outcome.interop_active);
        ftd::test::check("no false 'enabled after reload' log when it never was active",
                         !outcome.log_enabled);
        ftd::test::check("no 'lost interop' log when it was not active before either",
                         !outcome.log_lost);
    }

    ftd::test::section("an active->inactive transition must be reported");
    {
        const auto outcome = ftd::native::reimport_interop_after_reload(
            session, fake_buf, kFakeBytes, fake_fence, /*was_active=*/true);
        ftd::test::check("reimport still fails on the CPU backend",
                         !outcome.interop_active);
        ftd::test::check("log_lost fires on an active->inactive transition",
                         outcome.log_lost);
        ftd::test::check("log_enabled and log_lost are mutually exclusive here",
                         !outcome.log_enabled);
    }

    ftd::test::section(
        "missing buffer handle -- the shape a closed/never-supplied handle "
        "takes (see this file's header comment: a live GPU session is "
        "needed to prove the guard actually skips the call, but the "
        "outcome must be correct either way)");
    {
        const auto outcome = ftd::native::reimport_interop_after_reload(
            session, /*shared_buffer_handle=*/nullptr, kFakeBytes, fake_fence,
            /*was_active=*/true);
        ftd::test::check("a null buffer handle yields a failed reimport, not a crash",
                         !outcome.interop_active);
        ftd::test::check(
            "a null buffer handle after an active session still reports the "
            "active->inactive transition",
            outcome.log_lost);
    }

    ftd::test::section("missing fence handle is guarded the same way");
    {
        const auto outcome = ftd::native::reimport_interop_after_reload(
            session, fake_buf, kFakeBytes, /*shared_fence_handle=*/nullptr,
            /*was_active=*/false);
        ftd::test::check("a null fence handle yields a failed reimport",
                         !outcome.interop_active);
        ftd::test::check("no spurious logs when it was not active before",
                         !outcome.log_enabled && !outcome.log_lost);
    }

    ftd::test::section(
        "both handles missing (e.g. --cpu startup, or a session that never "
        "had interop) is a clean no-op, never a crash");
    {
        const auto outcome = ftd::native::reimport_interop_after_reload(
            session, nullptr, 0, nullptr, /*was_active=*/false);
        ftd::test::check("no handles at all: reimport reports inactive",
                         !outcome.interop_active);
        ftd::test::check("no handles at all: no logs fire",
                         !outcome.log_enabled && !outcome.log_lost);
    }

    return ftd::test::finalize();
}
