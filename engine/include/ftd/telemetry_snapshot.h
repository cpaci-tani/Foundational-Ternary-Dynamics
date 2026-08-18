#pragma once
/**
 * @file telemetry_snapshot.h
 * @brief Versioned, coherent observation snapshots for interactive engines.
 *
 * A telemetry snapshot is an observation of one settled engine epoch.  It is
 * deliberately separate from the renderer's bulk-field frames: callers stage
 * one requested set of scalar reductions, fence it, and publish only after the
 * corresponding epoch is complete.  This prevents a side panel from mixing a
 * diagnostics value from tick N with an energy value from tick N+1.
 *
 * The types in this header are CUDA-independent so Backend and RenderBridge
 * can expose the same contract to native GPU and CPU callers.  A group is
 * valid only when its bit is present in TelemetrySnapshot::groups.
 */

#include "ftd/render_bridge_diagnostics.h"

#include <cstdint>

namespace ftd {

enum TelemetryGroup : std::uint32_t {
    TELEMETRY_DIAGNOSTICS = 1u << 0,
    TELEMETRY_AUDIT       = 1u << 1,
    TELEMETRY_GRAVITY     = 1u << 2,
    TELEMETRY_LAGRANGIAN  = 1u << 3,
    TELEMETRY_ALL = TELEMETRY_DIAGNOSTICS | TELEMETRY_AUDIT
                  | TELEMETRY_GRAVITY | TELEMETRY_LAGRANGIAN,
};

/// Scheduler-owned observation request. `epoch` is opaque to the engine and
/// is echoed verbatim so a native publisher can reject stale completions.
struct TelemetrySnapshotRequest {
    std::uint32_t groups = TELEMETRY_DIAGNOSTICS;
    std::uint64_t epoch = 0;
    // Authoritative RenderBridge values stamped at submission time by the
    // Backend. They travel with the fence so an async completion never reads
    // a newer clock/timestep from the bridge.
    double physical_time = 0.0;
    double dt = 1.0;
    int lattice_size = 0;
};

/// CUDA-independent wire/cache representation of LagrangianDiag.  Keeping
/// this POD here avoids importing lagrangian.h (and its RenderBridge cycle)
/// into the core backend interface.
struct TelemetryLagrangian {
    double field_kinetic_sum = 0.0;
    double field_gradient_sum = 0.0;
    double born_infeld_sum = 0.0;
    double coupling_sum = 0.0;
    double velocity_coupling_sum = 0.0;
    double gauss_sum = 0.0;
    double dissipation_sum = 0.0;
    double total_lagrangian = 0.0;
    double total_hamiltonian = 0.0;
    double total_action = 0.0;
    double gauss_violation = 0.0;
    double max_gauss_error = 0.0;
    double total_flux_mag = 0.0;
    double total_wave_energy = 0.0;
    int manifested_count = 0;
    int locked_count = 0;
    double cell_volume = VOXEL_VOLUME;
};

/// Provenance for one telemetry group. A native publisher may retain a slow
/// group from an earlier snapshot while publishing a newer fast group, so
/// consumers must use the matching group meta rather than treating the
/// top-level tick as universal freshness.
struct TelemetryGroupMeta {
    std::uint64_t epoch = 0;
    std::uint64_t state_version = 0;
    int tick = 0;
    double physical_time = 0.0;
    double dt = 1.0;
    int lattice_size = 0;
};

/// Immutable scalar observation once poll_telemetry_snapshot() succeeds.
/// `state_version` increments for GPU-visible mutations. A value of zero means
/// that the backend has no mutation-generation counter (the CPU compatibility
/// backend); callers then use the scheduler epoch plus per-group tick.
/// `tick` is retained for existing protocol compatibility and need not change
/// for injections.
struct TelemetrySnapshot {
    std::uint64_t epoch = 0;
    std::uint64_t state_version = 0;
    int tick = 0;
    double physical_time = 0.0;
    double dt = 1.0;
    int lattice_size = 0;
    std::uint32_t groups = 0;
    Diagnostics diagnostics;
    EnergyAudit audit;
    GravityMetricAgg gravity;
    TelemetryLagrangian lagrangian;
    TelemetryGroupMeta diagnostics_meta;
    TelemetryGroupMeta audit_meta;
    TelemetryGroupMeta gravity_meta;
    TelemetryGroupMeta lagrangian_meta;
};

}  // namespace ftd
