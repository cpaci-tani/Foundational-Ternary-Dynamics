/**
 * @file backend.cpp
 * @brief Backend implementations — CpuBackend + GpuBackend.
 *
 * ARCH-2 Phases B/C (CHECKLIST_ENGINE.md): the wrappers delegate to the
 * existing RenderBridge phase methods (CPU) and GpuEngine entry points (GPU).
 * No behavior change — this is the abstraction layer the next migration
 * phases will dispatch through.
 */

#include "ftd/backend.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"   // phase_forces_integrate_clusters (GPU cluster-inertia mirror)
#include "ftd/eft/dual_cell_continuity.h"  // complete type for continuity_step (revision 3.1)
#include "ftd/lagrangian.h"
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

// ─── CpuBackend ───────────────────────────────────────────────────────────

CpuBackend::CpuBackend(RenderBridge& bridge) : bridge_(bridge) {}

void CpuBackend::tick() {
    // Currently the CPU tick body lives in RenderBridge::tick() and is gated
    // by the `use_gpu_` flag. Phase D (next) migrates that body here. For
    // now this method is unreachable — RenderBridge::tick() does the dispatch
    // via the existing if-ladder.
    //
    // Once Phase D lands: this calls bridge_.phase_read(); phase_write();
    // gauss_project(); phase_forces(); phase_movement(); etc.
}

namespace {

void copy_telemetry_lagrangian(const LagrangianDiag& source,
                               TelemetryLagrangian& target) {
    target.field_kinetic_sum = source.field_kinetic_sum;
    target.field_gradient_sum = source.field_gradient_sum;
    target.born_infeld_sum = source.born_infeld_sum;
    target.coupling_sum = source.coupling_sum;
    target.velocity_coupling_sum = source.velocity_coupling_sum;
    target.gauss_sum = source.gauss_sum;
    target.dissipation_sum = source.dissipation_sum;
    target.total_lagrangian = source.total_lagrangian;
    target.total_hamiltonian = source.total_hamiltonian;
    target.total_action = source.total_action;
    target.gauss_violation = source.gauss_violation;
    target.max_gauss_error = source.max_gauss_error;
    target.total_flux_mag = source.total_flux_mag;
    target.total_wave_energy = source.total_wave_energy;
    target.manifested_count = source.manifested_count;
    target.locked_count = source.locked_count;
    target.cell_volume = source.cell_volume;
}

VisualSnapshotRequest stamp_visual_request(const VisualSnapshotRequest& requested,
                                           const RenderBridge& bridge) {
    VisualSnapshotRequest request = requested;
    if (request.max_particles == 0) {
        request.max_particles = kMaxVisualParticleCapture;
    } else {
        request.max_particles = (std::min)(
            request.max_particles, kMaxVisualParticleCapture);
    }
    request.physical_time = bridge.physical_time();
    request.dt = bridge.dt();
    request.lattice_size = bridge.lattice().size();
    return request;
}

// Interior-occlusion test for the visual cull (VisualSnapshotRequest::
// interior_cull_layers). A manifested site is "buried" when, along all 6 axis
// directions, the next `layers` voxels are all manifested and in-bounds; such a
// site is fully occluded by the shell around it and is dropped from the visual
// gather. `layers <= 0` short-circuits to false (cull disabled → nothing buried,
// so the gather is bit-identical to before). Lattice index packing matches
// Lattice::index (x*L^2 + y*L + z), so coord decode is z=idx%L, y=(idx/L)%L,
// x=idx/L^2 — identical to the GPU interop_particle_gather_kernel.
template <typename VoxelVec>
bool visual_site_buried(const VoxelVec& voxels, int L, std::size_t idx, int layers) {
    if (layers <= 0 || L <= 0) return false;
    const std::size_t LL = static_cast<std::size_t>(L) * static_cast<std::size_t>(L);
    const int cz = static_cast<int>(idx % static_cast<std::size_t>(L));
    const int cy = static_cast<int>((idx / static_cast<std::size_t>(L))
                                    % static_cast<std::size_t>(L));
    const int cx = static_cast<int>(idx / LL);
    static const int D[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int d = 0; d < 6; ++d) {
        for (int s = 1; s <= layers; ++s) {
            const int nx = cx + D[d][0] * s;
            const int ny = cy + D[d][1] * s;
            const int nz = cz + D[d][2] * s;
            if (nx < 0 || nx >= L || ny < 0 || ny >= L || nz < 0 || nz >= L)
                return false;  // reached the lattice edge → surface, keep
            const std::size_t nidx =
                (static_cast<std::size_t>(nx) * static_cast<std::size_t>(L)
                 + static_cast<std::size_t>(ny)) * static_cast<std::size_t>(L)
                + static_cast<std::size_t>(nz);
            if (voxels[nidx].state == 0) return false;  // reached a void → keep
        }
    }
    return true;  // solid along all 6 axes for `layers` steps → buried, cull
}

void capture_cpu_particles(const RenderBridge& bridge,
                           const VisualSnapshotRequest& request,
                           VisualSnapshot& out) {
    out = {};
    out.kind = VisualCaptureKind::Particles;
    out.meta.epoch = request.epoch;
    // The CPU compatibility backend deliberately has no device mutation
    // generation.  Its caller uses epoch/tick provenance just as it does for
    // telemetry snapshots.
    out.meta.state_version = 0;
    out.meta.tick = bridge.current_tick();
    out.meta.physical_time = request.physical_time;
    out.meta.dt = request.dt;
    out.meta.lattice_size = request.lattice_size;

    const auto& voxels = static_cast<const RenderBridge&>(bridge).voxels();
    const int L = request.lattice_size;
    const int cull = static_cast<int>(request.interior_cull_layers);
    // `visible` is the gather pool: manifested sites that survive the interior
    // cull. With cull==0 it equals the true manifested count (unchanged path).
    std::uint64_t visible = 0;
    for (std::size_t index = 0; index < voxels.size(); ++index) {
        if (voxels[index].state == 0) continue;
        if (visual_site_buried(voxels, L, index, cull)) continue;
        ++visible;
    }
    out.particles.total_manifested = static_cast<std::uint32_t>(visible);
    const std::uint64_t selected = (std::min)(
        visible, static_cast<std::uint64_t>(request.max_particles));
    out.particles.records.reserve(static_cast<std::size_t>(selected));

    // This is intentionally the same Bresenham-style accumulator used by
    // ws_server's legacy pack_particle_data(): traverse the visible sites in
    // ascending lattice index and select an evenly-spaced deterministic subset
    // when the protocol cap is lower than the visible population.
    std::uint64_t accumulator = 0;
    for (std::size_t index = 0; index < voxels.size(); ++index) {
        const Voxel& voxel = voxels[index];
        if (voxel.state == 0) continue;
        if (visual_site_buried(voxels, L, index, cull)) continue;
        if (visible > selected) {
            accumulator += selected;
            if (accumulator < visible) continue;
            accumulator -= visible;
        }
        VisualParticleRecord record;
        record.index = static_cast<std::int32_t>(index);
        record.state = voxel.state;
        record.spin = voxel.spin;
        record.color = voxel.color;
        record.remainder_x = static_cast<float>(voxel.remainder.x);
        record.remainder_y = static_cast<float>(voxel.remainder.y);
        record.remainder_z = static_cast<float>(voxel.remainder.z);
        out.particles.records.push_back(record);
    }
}

}  // namespace

bool CpuBackend::begin_telemetry_snapshot(
    const TelemetrySnapshotRequest& requested) {
    if (telemetry_snapshot_pending_) return false;

    TelemetrySnapshotRequest request = requested;
    request.groups &= TELEMETRY_ALL;
    if (request.groups == 0) request.groups = TELEMETRY_DIAGNOSTICS;
    request.physical_time = bridge_.physical_time();
    request.dt = bridge_.dt();
    request.lattice_size = bridge_.lattice().size();

    // CPU is the compatibility path: take all requested values back-to-back
    // now, then expose them through the same poll contract as GPU. Native
    // callers never wait here when the active backend is CUDA.
    TelemetrySnapshot snapshot;
    // The CPU compatibility backend has no mutation-generation counter.
    // Do not substitute publication count for state_version: zero explicitly
    // tells schedulers to use epoch/tick provenance instead.
    const TelemetryGroupMeta meta{
        request.epoch, 0, bridge_.current_tick(),
        request.physical_time, request.dt, request.lattice_size};
    snapshot.epoch = meta.epoch;
    snapshot.state_version = meta.state_version;
    snapshot.tick = meta.tick;
    snapshot.physical_time = meta.physical_time;
    snapshot.dt = meta.dt;
    snapshot.lattice_size = meta.lattice_size;
    snapshot.groups = request.groups;
    if (request.groups & TELEMETRY_DIAGNOSTICS) {
        snapshot.diagnostics = bridge_.diagnostics();
        snapshot.diagnostics_meta = meta;
    }
    if (request.groups & TELEMETRY_AUDIT) {
        snapshot.audit = bridge_.energy_audit();
        snapshot.audit_meta = meta;
    }
    if (request.groups & TELEMETRY_GRAVITY) {
        snapshot.gravity = bridge_.gravity_metric_agg();
        snapshot.gravity_meta = meta;
    }
    if (request.groups & TELEMETRY_LAGRANGIAN) {
        copy_telemetry_lagrangian(compute_lagrangian_diagnostics(bridge_),
                                  snapshot.lagrangian);
        snapshot.lagrangian_meta = meta;
    }
    telemetry_snapshot_ = snapshot;
    telemetry_snapshot_pending_ = true;
    return true;
}

bool CpuBackend::telemetry_snapshot_ready() const {
    return telemetry_snapshot_pending_;
}

bool CpuBackend::poll_telemetry_snapshot(TelemetrySnapshot& out) {
    if (!telemetry_snapshot_pending_) return false;
    out = telemetry_snapshot_;
    telemetry_snapshot_pending_ = false;
    return true;
}

bool CpuBackend::begin_visual_snapshot(
    const VisualSnapshotRequest& requested) {
    if (visual_snapshot_pending_ || requested.kind != VisualCaptureKind::Particles) {
        return false;
    }
    const VisualSnapshotRequest request = stamp_visual_request(requested, bridge_);
    capture_cpu_particles(bridge_, request, visual_snapshot_);
    visual_snapshot_pending_ = true;
    return true;
}

bool CpuBackend::visual_snapshot_ready() const {
    return visual_snapshot_pending_;
}

bool CpuBackend::poll_visual_snapshot(VisualSnapshot& out) {
    if (!visual_snapshot_pending_) return false;
    out = std::move(visual_snapshot_);
    visual_snapshot_ = {};
    visual_snapshot_pending_ = false;
    return true;
}

// ─── GpuBackend ───────────────────────────────────────────────────────────

#ifdef FTD_ENABLE_CUDA
GpuBackend::GpuBackend(RenderBridge& bridge, gpu::GpuEngine* engine)
    : bridge_(bridge), engine_(engine) {}

void GpuBackend::tick() {
    // ARCH-2-D: full GPU tick body. Replaces the if-use_gpu_ block in
    // RenderBridge::tick(). Caller (RenderBridge::tick) is responsible for
    // toggle validation, post-tick proper-time accumulation, and ledger
    // update — those are backend-agnostic.
    if (!engine_) return;

    // knot_tracking still requires the canonical host AoS on every tick.
    // At large interactive lattice sizes that transfer is hundreds of MiB
    // and presents as an application freeze. Reject it before mutating
    // device state. cluster_inertia is a native 1-thread CUDA flood-fill.
    constexpr int MAX_INTERACTIVE_HOST_POSTPROCESS_L = 64;
    if (bridge_.interactive_gpu_mode_
        && bridge_.lattice_.size() > MAX_INTERACTIVE_HOST_POSTPROCESS_L
        && bridge_.toggles.knot_tracking) {
        throw std::logic_error(
            "[GpuBackend] knot_tracking is host-scoped above L=64 in "
            "interactive CUDA mode; reduce the lattice to L<=64 or disable "
            "the extension");
    }

    // Sync toggle state to the GPU engine each tick.
    engine_->toggles = bridge_.toggles;
    engine_->genesis_threshold_override = bridge_.genesis_threshold_override;
    engine_->manifest_scale_override = bridge_.manifest_scale_override;
    engine_->manifest_use_temperature = bridge_.manifest_use_temperature;
    // Wave 5.2 (2026-04-14): flush direct host writes back to device. Toggle
    // synchronization comes first so upload-side behavior observes the exact
    // term profile for this tick.
    flush_host_mutations();
    // Revision 0.9 option (a): prime the device gauge-link buffers on the
    // first su2_gauge/su3_gauge-enabled tick. The RenderBridge host arrays
    // are the canonical store (lazily materialized, revision 4.1b); the
    // upload happens ONCE on activation — host-side link mutations after
    // activation are not tracked (no engine path writes them; tests perturb
    // links before the first tick). Downloads happen in sync_to_host().
    if ((bridge_.toggles.su2_gauge || bridge_.toggles.su3_gauge)
        && !engine_->gauge_links_on_device()) {
        bridge_.ensure_gauge_links();
        engine_->upload_gauge_links(
            bridge_.su2_links_x_, bridge_.su2_links_y_, bridge_.su2_links_z_,
            bridge_.su3_links_x_, bridge_.su3_links_y_, bridge_.su3_links_z_);
    }
    // Mark before launch: even an exception raised while completing the tick
    // may follow kernels that already consumed identity IDs.  A later
    // synchronization boundary must still reconcile that high-water mark.
    identity_counters_dirty_ = true;
    engine_->tick();
    if (bridge_.toggles.matched_gauss_dynamics) {
        engine_->download_matched_gauss(*bridge_.matched_gauss_dynamics_);
    }
    if (bridge_.toggles.strong_stress_energy) {
        engine_->download_strong_step_diagnostics(bridge_.strong_energy_step_diag_);
    }
    bridge_.gpu_dirty_ = true;
    bridge_.physical_time_ += bridge_.dt_;
    ++bridge_.tick_;

    // Native tick_complete/run_complete is a real CUDA completion barrier.
    // The counter buffer is always allocated/reset, and this 8-byte D2H copy
    // synchronizes the serialized default stream while also surfacing launch
    // faults at the originating tick.  Previously movement=false skipped this
    // read, so large-L wave/static scenarios acknowledged queued kernels early
    // and the browser could enqueue work faster than CUDA completed it.
    const auto completed_causal_events = engine_->causal_projection_events();
    bridge_.causal_projection_events_this_tick_ = bridge_.toggles.movement
        ? static_cast<long long>(completed_causal_events) : 0;

    const bool requires_host_postprocess = bridge_.toggles.knot_tracking;
    if (bridge_.interactive_gpu_mode_ && !requires_host_postprocess) {
        // The native renderer uses compact state/flux readbacks. Keep the
        // canonical AoS shadow dirty until host-only diagnostics request it,
        // eliminating the former 469-byte/site transfer from every tick.
        return;
    }

    // Download device buffers so host-only extension passes and
    // update_energy_ledger() see fresh state.
    sync_to_host();
}

void GpuBackend::set_dt(double dt) {
    if (engine_) {
        // Sync toggles BEFORE set_dt so GpuEngine::set_dt sees the live
        // symplectic_leapfrog flag (it decides whether dt<1 is honored).
        // tick() re-syncs every tick, so this is only load-bearing when set_dt
        // is called during setup, before the first tick (the common case for
        // campaigns: rb.toggles.symplectic_leapfrog=true; rb.set_dt(0.5)).
        engine_->toggles = bridge_.toggles;
        engine_->set_dt(dt);
    }
}

void GpuBackend::sync_to_host() {
    // ARCH-2-A: download device buffers when host shadow is stale. Friend
    // access lets us touch RenderBridge's private sync state directly during
    // the migration phase.
    if (engine_ && bridge_.gpu_dirty_) {
        engine_->sync_to_host(bridge_.voxels_);
        bridge_.phi_          = engine_->phi();
        bridge_.phi_coulomb_  = engine_->phi_coulomb();
        bridge_.phi_latency_  = engine_->phi_latency();
        // Per-site force diagnostics — without this, GPU runs returned zero
        // from RenderBridge::force_diag_at() because the array was never
        // populated. The four tests test_emergent_measurements,
        // test_asymptotic_freedom, test_confinement, and test_triad_confinement
        // depend on this scatter to read f_coulomb / f_strong after tick().
        const auto& fd = engine_->force_diag();
        const int n = static_cast<int>(bridge_.force_diag_.size());
        for (int i = 0; i < n; ++i) {
            auto& d = bridge_.force_diag_[i];
            d.f_coulomb  = { fd.coulomb_x[i],  fd.coulomb_y[i],  fd.coulomb_z[i]  };
            d.f_strong   = { fd.strong_x[i],   fd.strong_y[i],   fd.strong_z[i]   };
            d.f_magnetic = { fd.magnetic_x[i], fd.magnetic_y[i], fd.magnetic_z[i] };
            d.f_gravity  = { fd.gravity_x[i],  fd.gravity_y[i],  fd.gravity_z[i]  };
            d.f_exchange = { fd.exchange_x[i], fd.exchange_y[i], fd.exchange_z[i] };
        }
        // Revision 0.9 option (a): mirror the relaxed gauge links back into
        // the RenderBridge host arrays so su2/su3_links_*() accessors (and
        // the CPU/GPU parity test) see the device state. No-op unless the
        // gauge sector was activated (buffers are lazy on both sides).
        if (engine_->gauge_links_on_device()) {
            engine_->download_gauge_links(
                bridge_.su2_links_x_, bridge_.su2_links_y_, bridge_.su2_links_z_,
                bridge_.su3_links_x_, bridge_.su3_links_y_, bridge_.su3_links_z_);
        }
        bridge_.sync_ternary_from_voxels();
        bridge_.sync_fields_from_voxels();
        bridge_.gpu_dirty_    = false;
    }
    if (engine_ && identity_counters_dirty_) {
        // Preserve lifetime identity monotonicity across GPU -> CPU fallback.
        // Live-voxel maxima are insufficient because evaporation/annihilation
        // clears provenance fields while the issued IDs must never be reused.
        int32_t next_particle_id = 0;
        int32_t next_pair_id = 0;
        engine_->identity_counters(next_particle_id, next_pair_id);
        bridge_.injector_.raise_identity_counters(next_particle_id,
                                                  next_pair_id);
        identity_counters_dirty_ = false;
    }
}

void GpuBackend::mark_host_dirty() {
    // ARCH-2-B: mark the host shadow as needing upload before next tick.
    bridge_.host_mutated_ = true;
}

void GpuBackend::push_to_device() {
    // ARCH-2-B: unconditional upload host→device (used by inject_*_cpu when
    // the GPU is active and the host has been edited directly).
    if (engine_) {
        // Raising the counters precedes the voxel upload, so conservatively
        // mark the compact mirror stale before either operation can throw.
        identity_counters_dirty_ = true;
        engine_->raise_identity_counters(
            bridge_.injector_.peek_next_particle_id(),
            bridge_.injector_.peek_next_pair_id());
        engine_->upload_from_host(bridge_.voxels_);
        bridge_.gpu_dirty_ = false;
    }
}

void GpuBackend::flush_host_mutations() {
    // ARCH-2-C: conditional upload — only when host_mutated_ was set since
    // the last flush. Wave 5.2 (2026-04-14): tick() calls this at the start
    // so direct host writes via voxels()[idx].field = ... reach the device.
    // Bug-fix 2026-04-21: also called from RenderBridge::run() GPU fast-path.
    if (engine_ && bridge_.host_mutated_) {
        identity_counters_dirty_ = true;
        engine_->raise_identity_counters(
            bridge_.injector_.peek_next_particle_id(),
            bridge_.injector_.peek_next_pair_id());
        engine_->upload_from_host(bridge_.voxels_);
        bridge_.gpu_dirty_   = false;
        bridge_.host_mutated_ = false;
    }
}

void GpuBackend::mirror_phi_latency() {
    // ARCH-2-F: lazy fetch of d_phi_latency into RenderBridge::phi_latency_
    // so external callers (phi_latency() accessor) get a stable host ref.
    if (engine_) {
        bridge_.phi_latency_ = engine_->phi_latency();
    }
}

void GpuBackend::mark_gpu_dirty() {
    // ARCH-2-I: indicate the device has newer state than the host shadow.
    // The next access through voxels()/voxel_at() will trigger sync_to_host.
    // Used by inject_*_cpu free functions after a GPU-side inject call.
    bridge_.gpu_dirty_ = true;
    identity_counters_dirty_ = true;
}

bool GpuBackend::copy_visual_states(std::vector<std::int8_t>& out) {
    if (!engine_) return false;
    engine_->copy_visual_states(out);
    return true;
}

bool GpuBackend::copy_visual_flux_magnitude(std::vector<float>& out) {
    if (!engine_) return false;
    engine_->copy_visual_flux_magnitude(out);
    return true;
}

bool GpuBackend::copy_visual_flux_magnitude_plane(
    int axis, int index, std::vector<float>& out) {
    if (!engine_) return false;
    engine_->copy_visual_flux_magnitude_plane(axis, index, out);
    return true;
}

bool GpuBackend::copy_visual_field_sample(VisualFieldKind kind, int stride,
                                          VisualFieldSample& out) {
    if (!engine_) return false;
    engine_->copy_visual_field_sample(kind, stride, out);
    return true;
}

bool GpuBackend::copy_visual_particle_attributes(
    const std::vector<int>& indices, std::vector<float>& out) {
    if (!engine_) return false;
    engine_->copy_visual_particle_attributes(indices, out);
    return true;
}

bool GpuBackend::copy_compact_diagnostics(Diagnostics& out) {
    if (!engine_ || !bridge_.interactive_gpu_mode_) return false;
    engine_->toggles = bridge_.toggles;
    out = engine_->diagnostics();
    return true;
}

bool GpuBackend::copy_compact_energy_audit(EnergyAudit& out) {
    if (!engine_ || !bridge_.interactive_gpu_mode_
        || bridge_.toggles.strong_stress_energy) return false;
    engine_->toggles = bridge_.toggles;
    out = engine_->energy_audit();
    return true;
}

bool GpuBackend::copy_compact_gravity_metric(GravityMetricAgg& out) {
    if (!engine_ || !bridge_.interactive_gpu_mode_) return false;
    engine_->toggles = bridge_.toggles;
    out = engine_->gravity_metric_agg();
    return true;
}

bool GpuBackend::copy_compact_lagrangian(LagrangianDiag& out) {
    if (!engine_ || !bridge_.interactive_gpu_mode_) return false;
    engine_->toggles = bridge_.toggles;
    engine_->lagrangian_diagnostics(out);
    return true;
}

bool GpuBackend::copy_compact_voxel(int index, VoxelInspection& out) {
    if (!engine_ || !bridge_.interactive_gpu_mode_) return false;
    engine_->toggles = bridge_.toggles;
    engine_->inspect_voxel(index, out);
    return true;
}

bool GpuBackend::copy_compact_force(int index, ForceDiag& out) {
    if (!engine_ || !bridge_.interactive_gpu_mode_) return false;
    engine_->inspect_force(index, out);
    return true;
}

bool GpuBackend::begin_telemetry_snapshot(
    const TelemetrySnapshotRequest& request) {
    if (!engine_ || !bridge_.interactive_gpu_mode_) return false;
    // Mirror the tick boundary ordering: toggle/override state and any direct
    // host mutation are committed before we stamp the GPU observation epoch.
    engine_->toggles = bridge_.toggles;
    engine_->genesis_threshold_override = bridge_.genesis_threshold_override;
    engine_->manifest_scale_override = bridge_.manifest_scale_override;
    engine_->manifest_use_temperature = bridge_.manifest_use_temperature;
    flush_host_mutations();
    TelemetrySnapshotRequest stamped = request;
    stamped.physical_time = bridge_.physical_time();
    stamped.dt = bridge_.dt();
    stamped.lattice_size = bridge_.lattice().size();
    if (!engine_->begin_telemetry_snapshot(stamped)) return false;
    telemetry_snapshot_self_field_injection_ = bridge_.self_field_injection_;
    return true;
}

bool GpuBackend::telemetry_snapshot_ready() const {
    return engine_ && engine_->telemetry_snapshot_ready();
}

bool GpuBackend::poll_telemetry_snapshot(TelemetrySnapshot& out) {
    if (!engine_ || !engine_->poll_telemetry_snapshot(out)) return false;
    // This member lives in RenderBridge because it is a ledger value, not a
    // field reduction. Preserve the same stitching as energy_audit().
    if (out.groups & TELEMETRY_AUDIT) {
        out.audit.self_field_injection = telemetry_snapshot_self_field_injection_;
    }
    return true;
}

bool GpuBackend::begin_visual_snapshot(
    const VisualSnapshotRequest& requested) {
    if (!engine_ || !bridge_.interactive_gpu_mode_
        || requested.kind != VisualCaptureKind::Particles) {
        return false;
    }
    // Direct editor/scenario writes must be committed before the device scan
    // so every captured record and provenance field observes one source epoch.
    flush_host_mutations();
    return engine_->begin_visual_snapshot(stamp_visual_request(requested, bridge_));
}

bool GpuBackend::visual_snapshot_ready() const {
    return engine_ && engine_->visual_snapshot_ready();
}

bool GpuBackend::poll_visual_snapshot(VisualSnapshot& out) {
    return engine_ && engine_->poll_visual_snapshot(out);
}

bool GpuBackend::visual_snapshot_safe_to_replace() const {
    return !engine_ || engine_->visual_snapshot_safe_to_replace();
}

bool GpuBackend::visual_snapshot_in_flight() const {
    return engine_ && engine_->visual_snapshot_in_flight();
}

void GpuBackend::set_rng_seed(unsigned int seed) {
    // Revision 3.1 (was an ifdef in RenderBridge::seed_rng): cuRAND picks the
    // seed up on the next device tick.
    if (engine_) engine_->set_rng_seed(seed);
}

bool GpuBackend::continuity_step(eft::DualCellContinuity& out) {
    // Revision 3.1 (was an ifdef in RenderBridge::continuity_step).
    if (!engine_) return false;
    out = engine_->continuity_step();
    return true;
}
#endif

// ─── Default backend factory (revision 3.1) ────────────────────────────────
// Owns the LAST backend-selection ifdef: GPU-default when CUDA is compiled in
// (ARCH-2 policy), CPU otherwise. RenderBridge's constructor is now
// ifdef-free apart from the gpu_engine.h include its defaulted destructor
// needs for complete-type unique_ptr deletion.
std::unique_ptr<Backend> make_default_backend(RenderBridge& bridge, int lattice_size) {
#ifdef FTD_ENABLE_CUDA
    if (const char* p = std::getenv("FTD_FORCE_CPU"); p && *p && *p != '0') {
        std::fprintf(stderr, "[RenderBridge] CPU backend (FTD_FORCE_CPU, L=%d)\n",
                     lattice_size);
        return std::make_unique<CpuBackend>(bridge);
    }
    bridge.gpu_ = std::make_unique<gpu::GpuEngine>(lattice_size);
    std::fprintf(stderr, "[RenderBridge] GPU backend active (CUDA, L=%d)\n", lattice_size);
    return std::make_unique<GpuBackend>(bridge, bridge.gpu_.get());
#else
    (void)lattice_size;
    return std::make_unique<CpuBackend>(bridge);
#endif
}

}  // namespace ftd
