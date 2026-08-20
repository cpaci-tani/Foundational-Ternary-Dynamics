#pragma once
//
// host/adapters/scale0_adapter.h — Scale 0 (voxel field, RenderBridge) behind
// the ScaleAdapter seam (SPEC_NATIVE_REBUILD_R0R1 §2.2, §4.1).
//
// This is the OLD NativeEngineSession/command_applier/ui_snapshot_builder logic
// re-homed. It owns the RenderBridge + the telemetry scheduler + the interop
// state, and dispatches every Scale-0 mutation/observation through the existing,
// golden-covered free functions (apply_mutation_on_bridge / observe_on_bridge /
// build_snapshot) — so behavior is identical to the session it replaces.
//
// This is the ONLY file in the host layer allowed to name concrete Scale-0
// types (RenderBridge, TermToggles, the Scale-0 command/snapshot structs).
//
#include "native/command_applier.h"      // UiBoundaryState + apply/observe free fns
#include "native/host/scale_adapter.h"
#include "native/model/snapshot.h"        // Scale0Snapshot
#include "native/parameter_journal.h"
#include "native/ui_snapshot_builder.h"

#include "ftd/native_telemetry_scheduler.h"  // value member (pulls render_bridge.h)

#include <cstdint>
#include <memory>
#include <string>

namespace ftd {
class RenderBridge;
#ifdef FTD_ENABLE_CUDA
namespace gpu { class GpuEngine; }
#endif
}

namespace ftd::native {

class Scale0Adapter final : public ScaleAdapter {
public:
    Scale0Adapter();
    ~Scale0Adapter() override;

    int scale_level() const override { return 0; }
    const char* scale_name() const override { return "Lattice"; }
    // RenderBridge does not derive from ScaleEngine — see scale_adapter.h.
    ftd::ScaleEngine* engine() override { return nullptr; }

    void boot(const ftd::ScenarioMeta& meta, const RunConfig& cfg,
              BootReport& out) override;

    void bind_sim_thread() override;
    void tick() override;
    int  current_tick() const override;

    bool is_observation(const ScalePayload& payload) const override;
    bool is_host_write(const ScalePayload& payload) const override;
    ApplyResult apply(const ScalePayload& payload, ParameterJournal& journal,
                      int apply_tick, LoopControl& loop) override;
    void flush_writes() override;

    void begin_boundary() override;
    bool observe(const ScalePayload& payload) override;
    void on_tick_complete() override;
    void build_snapshot(const DataNeeds& needs) override;
    ScaleSnapshot take_scale_snapshot() override;

    NativeFrame capture() override;

    const char* backend_name() const override;
    std::string status() const override { return status_; }
    std::string scenario_id() const override { return scenario_; }
    int lattice_size() const override;
    std::uint32_t last_total_manifested() const override { return last_total_manifested_; }

    bool try_enable_interop(void* buf, std::uint64_t bytes, void* fence) override;
    bool interop_enabled() const override { return interop_enabled_; }
    bool request_interop_gather(std::uint64_t fence_value) override;
    int  poll_interop_particle_count() override;

private:
    void apply_boundary();

    RunConfig                     cfg_;
    std::string                   scenario_;
    std::string                   status_;
    int                           flux_boundary_ = 2;
    std::unique_ptr<RenderBridge> bridge_;
    ftd::NativeTelemetryScheduler scheduler_;
    UiBoundaryState               obs_state_;          // observe deferral + demand
    Scale0Snapshot                boundary_snapshot_;  // accumulator for the boundary
    bool                          interop_enabled_ = false;
    std::uint32_t                 last_total_manifested_ = 0;
};

}  // namespace ftd::native
