#pragma once
//
// host/scale_host.h — the scale-generic session host (SPEC_NATIVE_REBUILD_R0R1 §4.1).
//
// ScaleHost owns the boot/reload/tick/drain/publish loop and the interop
// lifecycle for WHATEVER scale is active, holding it behind a ScaleAdapter. No
// concrete scale type (the Scale-0 engine, its toggle struct, or any Scale-0
// command/snapshot type) is named in this header or scale_host.cpp — the
// boundary check proves it; the only place that knows those types is the
// Scale-0 adapter. There is deliberately NO engine accessor (closes R-NOW-4):
// callers reach physics only through the published HostSnapshot and the
// captured NativeFrame.
//
// OWNERSHIP (seam note): the spec pictures the host owning a
// `unique_ptr<ScaleEngine>` beside the adapter. Scale 0's engine does not derive
// from ScaleEngine, so the adapter owns its concrete engine and the host owns
// only the adapter. See scale_adapter.h.
//
#include "native/host/command_bus.h"
#include "native/host/run_config.h"
#include "native/host/scale_adapter.h"
#include "native/host/snapshot_bus.h"
#include "native/parameter_journal.h"
#include "native/ui_demand.h"
#include "native/ui_result.h"

#include <cstdint>
#include <memory>
#include <string>

namespace ftd::native {

struct HostOptions {
    int         scale_level = 0;
    std::string scenario    = "s0-seed-hydrogen";
    RunConfig   run;
};

class ScaleHost {
public:
    explicit ScaleHost(HostOptions options);
    ~ScaleHost();

    ScaleHost(const ScaleHost&) = delete;
    ScaleHost& operator=(const ScaleHost&) = delete;

    // ── sim-thread drive ──
    TickResult tick_once();
    TickResult process_ui_boundary(CommandBus& bus);
    void       consume_pending_step();

    LoopControl loop_control() const { return loop_; }
    void        set_loop_control(LoopControl loop) { loop_ = loop; }

    NativeFrame capture();

    // ── scale switch (teardown interop, build engine+adapter, boot) ──
    ReloadResult switch_scale(int scale_level, std::string scenario, const RunConfig& cfg);

    // ── published state / bookkeeping ──
    SnapshotBus&      publisher() { return publisher_; }
    ParameterJournal& journal() { return journal_; }
    int  active_scale() const { return active_scale_; }
    bool applied_reload() const { return applied_reload_; }
    bool applied_host_write() const { return applied_host_write_; }
    ReloadResult last_reload_result() const { return last_reload_; }

    // ── common-core metadata (mirrors the published HostSnapshot core) ──
    const char* backend_name() const;
    std::string status() const;
    std::string scenario() const;
    int         lattice_size() const;

    // ── interop lifecycle (generalized W15; delegated to the active adapter) ──
    bool try_enable_interop(void* buf, std::uint64_t bytes, void* fence);
    bool interop_enabled() const;
    bool request_interop_gather(std::uint64_t fence_value);
    int  poll_interop_particle_count();

private:
    void reload_to(const std::string& scenario_id, const RunConfig& cfg,
                   ReloadResult& out);
    void publish_boundary();

    HostOptions                   options_;
    std::unique_ptr<ScaleAdapter> adapter_;
    SnapshotBus                   publisher_;
    ParameterJournal              journal_;

    LoopControl   loop_;
    DataNeeds     demand_;
    std::uint64_t snapshot_seq_ = 0;
    std::uint64_t last_applied_seq_ = 0;
    bool          did_tick_ = false;
    bool          applied_reload_ = false;
    bool          applied_host_write_ = false;
    bool          interop_was_active_ = false;
    ReloadResult  last_reload_;
    TickResult    last_tick_;
    int           active_scale_ = 0;
};

}  // namespace ftd::native
