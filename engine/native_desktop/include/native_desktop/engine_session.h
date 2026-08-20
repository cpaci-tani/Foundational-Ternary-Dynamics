#pragma once

#include "native_desktop/native_frame.h"
#include "native_desktop/command_applier.h"
#include "native_desktop/command_queue.h"
#include "native_desktop/parameter_journal.h"
#include "native_desktop/snapshot_publisher.h"
#include "native_desktop/ui_result.h"

#include "ftd/native_telemetry_scheduler.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ftd {
class RenderBridge;
#ifdef FTD_ENABLE_CUDA
namespace gpu {
class GpuEngine;
}
#endif
}

namespace ftd::native_desktop {

struct NativeEngineOptions {
    int lattice_size = 32;
    std::string scenario = "s0-seed-hydrogen";
    bool force_cpu = true;
    // Matches the web Scale-0 toolbar default (Dispersal).
    int flux_boundary = 2;
};

class NativeEngineSession {
public:
    explicit NativeEngineSession(NativeEngineOptions options);
    ~NativeEngineSession();

    NativeEngineSession(const NativeEngineSession&) = delete;
    NativeEngineSession& operator=(const NativeEngineSession&) = delete;

    void tick();
    TickResult tick_once();
    TickResult process_ui_boundary(CommandQueue& queue);
    void consume_pending_step();
    LoopControl loop_control() const { return ui_.loop; }
    void set_loop_control(LoopControl loop) { ui_.loop = loop; }
    void stage_lattice_size(int n) { staged_lattice_size_ = n; }
    int staged_lattice_size() const { return staged_lattice_size_; }
    void set_last_reload(ReloadResult result) { ui_.last_reload = std::move(result); }
    ReloadResult last_reload_result() const { return ui_.last_reload; }
    SnapshotPublisher& snapshot_publisher() { return publisher_; }
    ParameterJournal& parameter_journal() { return journal_; }
    NativeFrame capture();
    NativeFrame capture_particles() { return capture(); }

    void apply_options(NativeEngineOptions options);
    void load_scenario(std::string name);
    void set_lattice_size(int lattice_size);
    void set_flux_boundary(int flux_boundary);
    void reset_current();

    int lattice_size() const;
    int current_tick() const;
    int flux_boundary() const { return options_.flux_boundary; }
    const char* backend_name() const;
    const std::string& scenario() const { return options_.scenario; }
    const std::string& status() const { return status_; }
    const NativeEngineOptions& options() const { return options_; }

    // Thread affinity (single-writer-then-handoff, not enforced by this
    // class -- callers must honor it): try_enable_interop() is safe to call
    // from whichever thread currently exclusively owns `bridge_` -- the
    // main/GUI thread before the sim thread is constructed (the current
    // native_desktop main.cpp's startup call), OR the sim thread itself
    // afterward, once a reload's boot() has finished rebuilding `bridge_`
    // (main.cpp's do_reload branch re-imports the same still-open D3D12
    // buffer/fence NT handles there to re-establish interop after every
    // reload -- see Interop Task 12). It must never be called from both
    // "sides" concurrently. interop_enabled(), request_interop_gather(),
    // and poll_interop_particle_count() are safe to call only from the sim
    // thread once it exists -- same as tick(), capture(), and
    // apply_options(), which carry this same implicit contract because they
    // all reach into `bridge_`, and boot() (invoked by apply_options()) can
    // reset `bridge_` to null mid-reconstruction with zero locking. A call
    // racing a reload from any other thread is a null-pointer dereference
    // or a torn read, not a defined error.
    //
    // Attempts to initialize the D3D12/CUDA interop path: imports the given
    // shared buffer and fence handles into the GPU backend. No-op (returns
    // false) on the CPU backend or if either import fails -- callers must
    // keep using the plain capture()/render() path in that case, exactly as
    // before this method existed. Safe to call more than once against the
    // SAME session, including after a reload (apply_options()/
    // load_scenario()/set_lattice_size()/reset_current() all clear
    // interop_enabled_ via boot() -- see boot()'s doc comment): boot()
    // fully destroys the old bridge_/GpuEngine and constructs a fresh one
    // before this can run again, so there is no dangling-import or
    // re-entrancy-leak concern the way there would be re-importing into a
    // GpuEngine instance that was never destroyed. The NT handles
    // themselves may be reused across such calls -- neither
    // import_d3d12_particle_buffer() nor import_d3d12_fence() take
    // ownership of them (see gpu_engine.h) -- but the D3D12-side shared
    // fence's completed value is NOT reset by any of this (the underlying
    // ID3D12Fence object, unlike bridge_/GpuEngine, is never recreated
    // across a reload if the caller keeps reusing the same fence handle):
    // whatever fence_value a caller signals after a reload must keep
    // increasing from whatever value that same fence last reached before
    // the reload, or the D3D12-side Wait() the caller pairs it with can
    // return immediately without actually waiting for the new gather to
    // finish (cudaSignalExternalSemaphoresAsync itself will also simply
    // fail for a non-monotonic value -- see interop_signal_fence()'s doc
    // comment in gpu_engine.h).
    bool try_enable_interop(void* shared_buffer_handle, std::uint64_t buffer_bytes,
                            void* shared_fence_handle);
    bool interop_enabled() const { return interop_enabled_; }
    // Runs the interop gather (device-side only, no host particle vector) and
    // returns the particle count once ready, or -1 if not yet ready this call
    // (poll again). fence_value must be a strictly increasing counter the
    // caller also passes to D3D12Presenter::wait_shared_fence with the same
    // value.
    int poll_interop_particle_count();
    // Returns false if the underlying GpuEngine::interop_gather_particles()
    // call reports failure (interop disabled/no engine, or a real
    // interop_signal_fence() failure -- the gather may have succeeded but
    // the cross-API fence handoff did not, so the whole call is reported as
    // failed). Callers must treat false as "the D3D12 side cannot safely
    // consume this buffer" and fall back accordingly (see main.cpp's sim
    // thread loop).
    bool request_interop_gather(std::uint64_t fence_value);

    // TEST-ONLY. Exposes the underlying GPU engine for direct verification
    // (e.g. debug_read_interop_records). Production code never needs this --
    // it goes through capture()/tick() only.
#ifdef FTD_ENABLE_CUDA
    ftd::gpu::GpuEngine* debug_gpu_engine();
#endif
    // TEST-ONLY. Production code uses tick()/capture()/process_ui_boundary().
    RenderBridge& debug_bridge() { return *bridge_; }
    const RenderBridge& debug_bridge() const { return *bridge_; }

private:
    void boot();
    void apply_boundary();
    void fill_frame_meta(NativeFrame& frame) const;

    NativeEngineOptions options_;
    std::unique_ptr<RenderBridge> bridge_;
    std::string status_;
    bool interop_enabled_ = false;
    int staged_lattice_size_ = 0;
    SnapshotPublisher publisher_;
    ParameterJournal journal_;
    ftd::NativeTelemetryScheduler scheduler_;
    UiBoundaryState ui_;
};

// Outcome of one reimport_interop_after_reload() call -- see that function's
// doc comment for the full contract. `interop_active` is the value the
// caller should store into its own interop-active flag; `log_enabled` and
// `log_lost` are mutually exclusive (never both true) hints for which
// one-line console message, if either, the caller should print. Splitting
// these out as data lets a caller like main.cpp's do_reload branch be a
// thin, no-decision-logic-of-its-own call site: every branch that used to be
// inline there (guard against a missing handle, decide whether this reload
// crossed an active/inactive transition worth logging) now lives here,
// where it has ctest coverage (test_interop_reload_orchestration.cpp,
// test_interop_reload_reset.cpp) instead of none.
struct InteropReloadOutcome {
    bool interop_active = false;
    bool log_enabled = false;
    bool log_lost = false;
};

// Re-establishes D3D12/CUDA interop against `session` immediately after a
// reload (apply_options()/load_scenario()/set_lattice_size()/
// reset_current() -- every one of which funnels through boot()) has finished
// rebuilding session's internal bridge_/GpuEngine. This is the Interop
// Task 12 fix: boot() unconditionally clears interop_enabled_ on every
// reload (nothing has been imported into the freshly-constructed GpuEngine
// yet), so the caller must re-supply the SAME still-open D3D12 buffer/fence
// NT handles it used for the original import.
//
// This function does not create, duplicate, or close any handles -- it only
// re-imports the ones it is given, via try_enable_interop(). Passing a null
// shared_buffer_handle or shared_fence_handle (e.g. because a caller
// mistakenly closed them right after the very first import -- Interop
// Task 12's actual pre-fix bug; see commit 93d03a3c's message) intentionally
// short-circuits to a failed outcome instead of calling try_enable_interop()
// with a dangling handle.
//
// `was_active` is the caller's own interop-active flag value from
// immediately before this reload started (i.e. captured before
// apply_options()/load_scenario()/etc. ran); it controls only which (if
// either) of InteropReloadOutcome's log_* flags comes back true, never
// whether re-import is attempted.
//
// Thread affinity: identical to try_enable_interop() -- call only from
// whichever thread currently exclusively owns session's bridge_ (in
// main.cpp's usage: the sim thread, immediately after apply_options()/
// load_scenario() has already run on that same thread).
InteropReloadOutcome reimport_interop_after_reload(
    NativeEngineSession& session, void* shared_buffer_handle,
    std::uint64_t buffer_bytes, void* shared_fence_handle, bool was_active);

}  // namespace ftd::native_desktop
