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
#include "native/scale0_overlays.h"        // OverlayId + the overlay registry
#include "native/ui_snapshot_builder.h"

#include "ftd/native_telemetry_scheduler.h"  // value member (pulls render_bridge.h)
#include "ftd/visual_field_sample.h"          // VisualFieldKind (overlay member)

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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

    void boot(const ScenarioMeta& meta, const RunConfig& cfg,
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

    // Active overlay SET (SetOverlay toggles membership). Empty → capture()
    // emits the ambient flux cloud (unchanged base view). Otherwise capture()
    // composites EVERY active overlay into the frame (multiple arrow/point
    // groups coexist). Kept as a small ordered vector: membership is O(active),
    // dedup on insert, and the append order is deterministic. Mirrors the web
    // `anyFieldActive` gate on the ambient cloud.
    std::vector<OverlayId>        active_overlays_;

    // Per-active-sheet slice height (fraction of the lattice box). Keyed by the
    // numeric OverlayId. Seeded to the registry y_frac when a Sheet overlay is
    // toggled on; erased on toggle-off. SetSheetHeight updates it. build_sheet
    // slices the field on the y = height·L plane and sits the surface there, so
    // sweeping the height reads the energy at successive levels.
    std::unordered_map<std::uint32_t, float> sheet_height_;

    void set_overlay(OverlayId id, bool on);
    bool overlay_active(OverlayId id) const;
    // View-state height control (clamped to [0, 0.999]); no bridge mutation.
    void  set_sheet_height(OverlayId id, float height);
    // Current slice height for a sheet overlay — the stored value, or the
    // descriptor's y_frac default if none is stored yet.
    float sheet_height_frac(OverlayId id, const OverlayDescriptor& d) const;
    // Latency overlays (Latency L, Horizon) sample the real Poisson latency
    // (kind 17) instead of the normalized-|J|² proxy (kind 8) in native
    // mass-gravity scenarios, mirroring the web scale0FieldKindOverrides.
    ftd::VisualFieldKind resolve_overlay_kind(const OverlayDescriptor& d) const;
};

}  // namespace ftd::native
