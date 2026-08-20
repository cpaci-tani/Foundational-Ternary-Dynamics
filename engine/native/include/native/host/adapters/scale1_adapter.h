#pragma once
//
// host/adapters/scale1_adapter.h — Scale 1 (ParticleEngine) behind the
// ScaleAdapter seam (SPEC_NATIVE_REBUILD_R0R1 §4.1; R1 validation that the seam
// is scale-generic — a SECOND adapter with a structurally different engine).
//
// Unlike Scale 0 (RenderBridge, which predates ScaleEngine and so returns a null
// engine()), ParticleEngine IS a ftd::ScaleEngine, so this adapter fits the §4.1
// picture exactly: it owns the concrete engine and CAN hand a live engine()
// pointer up the seam. Scale 1 has no CUDA<->D3D12 interop path yet and needs no
// telemetry scheduler — the Scale1Snapshot is filled directly from
// ParticleEngine::diagnostics().
//
// This is the ONLY host-layer file besides scale0_adapter.{h,cpp} allowed to
// name a concrete engine type; the boundary check proves scale_host.{h,cpp}
// stays free of ParticleEngine / Scale1.
//
#include "native/host/scale_adapter.h"
#include "native/model/snapshot.h"   // Scale1Snapshot
#include "native/parameter_journal.h"

#include <cstdint>
#include <memory>
#include <string>

namespace ftd {
class ParticleEngine;
}

namespace ftd::native {

class Scale1Adapter final : public ScaleAdapter {
public:
    Scale1Adapter();
    ~Scale1Adapter() override;

    int scale_level() const override { return 1; }
    const char* scale_name() const override { return "Particles"; }
    // ParticleEngine derives from ScaleEngine — the seam's live-engine case.
    // Defined out-of-line (needs the complete type for the base-class upcast).
    ftd::ScaleEngine* engine() override;

    void boot(const ftd::ScenarioMeta& meta, const RunConfig& cfg,
              BootReport& out) override;

    void bind_sim_thread() override {}   // ParticleEngine has no sim-thread affinity
    void tick() override;
    int  current_tick() const override;

    bool is_observation(const ScalePayload& payload) const override;
    bool is_host_write(const ScalePayload& payload) const override;
    ApplyResult apply(const ScalePayload& payload, ParameterJournal& journal,
                      int apply_tick, LoopControl& loop) override;
    void flush_writes() override {}      // no deferred host-mutation queue at Scale 1

    void begin_boundary() override;
    bool observe(const ScalePayload& payload) override;
    void on_tick_complete() override {}  // no telemetry scheduler to pump
    void build_snapshot(const DataNeeds& needs) override;
    ScaleSnapshot take_scale_snapshot() override;

    NativeFrame capture() override;

    const char* backend_name() const override { return "cpu"; }
    std::string status() const override { return status_; }
    std::string scenario_id() const override { return scenario_; }
    int lattice_size() const override { return box_; }
    std::uint32_t last_total_manifested() const override { return last_count_; }

    // Interop methods stay the no-op defaults (no CUDA<->D3D12 path for Scale 1).

private:
    void seed_scenario(const std::string& id);

    std::string                          scenario_;
    std::string                          status_;
    int                                  box_ = 32;  // nominal render box → camera framing
    std::unique_ptr<ftd::ParticleEngine> engine_;
    Scale1Snapshot                       snapshot_;  // boundary accumulator
    std::uint32_t                        last_count_ = 0;
};

}  // namespace ftd::native
