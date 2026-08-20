/**
 * Logic-First FTD Engine (v2.0)
 *
 * Built from axioms: {3D lattice, ternary states, flux field, local causality}
 *
 * Six rules, nothing else:
 *   1. Flux wave equation: d²J/dt² = c²∇²J (local linear dynamics)
 *   2. State-flux coupling: −g_c·∇(s) source term (from δS/δJ = 0; electric
 *      sign per lagrangian.h Term 2, amended 2026-07-18)
 *   3. Gauss projection: enforce ∇·J = s (charge conservation)
 *   4. Manifestation/Evaporation: threshold crossing
 *   5. Field-mediated forces: F = -α·s·∇φ_C + G_N·∇ρ (Poisson Coulomb, Phase 3)
 *   6. Movement + Collision: remainder accumulation, speed limit, annihilation
 *
 * Everything phenomenological has been stripped:
 *   - No pairwise Coulomb, Yukawa, Lorentz, exchange forces
 *   - No QCD running coupling
 *   - No weak transmutation
 *   - No binding energy maintenance
 *   - No noetic/reference frame context
 *   - No latency/bandwidth/proper time
 *
 * What emerges from these rules IS the physics.
 * What doesn't emerge is a genuine absence, not a missing formula.
 *
 * Archived: engine_v1_phenomenological/ contains the full 1382-line version.
 */

#include "ftd/render_bridge.h"
#include "ftd/poisson_solvers.h"
#include "ftd/diagnostics_compute.h"
#include "ftd/injection.h"
#include "ftd/sublattice.h"
#include "ftd/transmutation_phases.h"      // moved from mid-file to avoid nested-namespace include
#include "ftd/energy_ledger_compute.h"     // moved from mid-file to avoid nested-namespace include
#include "ftd/render_bridge_phases.h"      // Phase 4a: phase_write decomposition (2026-04-27)
#include "ftd/knot_telemetry.h"            // Observation-only per-knot telemetry (PIMPL; complete type here)
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

#ifdef _OPENMP
#include <omp.h>
#endif
#include "ftd/parallel.h"

#ifdef FTD_ENABLE_CUDA
// The ONLY remaining CUDA conditional in this TU (revision 3.1): the
// defaulted ~RenderBridge() destroys unique_ptr<gpu::GpuEngine> gpu_, which
// requires the complete type here. Everything behavioral dispatches through
// the Backend virtuals; the selection policy lives in backend.cpp
// (make_default_backend).
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

void RenderBridge::bind_sim_thread() {
#ifndef NDEBUG
    sim_thread_ = std::this_thread::get_id();
    sim_thread_bound_ = true;
#else
    (void)sim_thread_;
    (void)sim_thread_bound_;
#endif
}

void RenderBridge::assert_sim_thread() const {
#ifndef NDEBUG
    if (!sim_thread_bound_) return;
    if (std::this_thread::get_id() != sim_thread_) {
        throw std::logic_error("FTD_UI_DEBUG_THREAD_GUARD");
    }
#endif
}

// F-13 (2026-04-27): VoxelRng salt domains and voxel_uniform() — the
// per-voxel deterministic uniform sampler used by genesis/evaporation —
// were moved to engine/src/render_bridge_phases/phase_write.cpp during
// the Phase 4a refactor (the only call sites were inside phase_write).
// Keep the salt integer values (1,2,3) stable in that TU: they are part
// of the public RNG stream definition.

RenderBridge::RenderBridge(int lattice_size)
    : lattice_(lattice_size), engine_state_(lattice_.total_sites()),
      voxels_(lattice_.total_sites()),
      force_diag_(lattice_.total_sites()),
      delta_j_(lattice_.total_sites()),
      delta_j_L_(lattice_.total_sites()),
      delta_j_R_(lattice_.total_sites()),
      dJ_(lattice_.total_sites()),
      phi_(lattice_.total_sites(), 0.0),
      phi_coulomb_(lattice_.total_sites(), 0.0),
      phi_latency_(lattice_.total_sites(), 0.0),
      moved_(lattice_.total_sites(), 0),
      sor_source_(lattice_.total_sites(), 0.0)
      // SU(2)/SU(3) link buffers are NOT allocated here (revision 4.1b):
      // 528 B/site (~132 MiB at L=64, larger than the voxel array) for a
      // toggle-gated sector. ensure_gauge_links() materializes them on the
      // first accessor call, relax call, or su2_gauge/su3_gauge-gated tick.
{
    // PERF: pre-size per-tick scratch buffers so phase_write doesn't
    // construct ~5KB of mt19937 state per voxel. Under WASM (no OpenMP)
    // num_threads is always 1; native builds size to omp_get_max_threads().
    int num_threads = 1;
#ifdef _OPENMP
    num_threads = omp_get_max_threads();
#endif
    thread_seeds_.resize(num_threads, 0u);
    rng_state_ = std::make_unique<BridgeRng>(42u);
    rng_state_->resize_thread_pool(static_cast<std::size_t>(num_threads));
    // Observation-only per-knot telemetry recorder (PIMPL; complete type via
    // the knot_telemetry.h include above). Only used when toggles.knot_tracking
    // is enabled. Golden-neutral (reads settled state).
    // min_cluster_size = 1 so EVERY manifested s≠0 voxel/cluster (including
    // isolated single-voxel charges) is tracked as a knot. NOTE: the
    // ClusterTrackerParams default stays 4 (the unit test relies on it).
    ClusterTrackerParams kt_params;
    kt_params.min_cluster_size = 1;
    knot_tracker_ = std::make_unique<KnotTracker>(kt_params);
    // FTD-HISTORY-BEGIN: observation-only native event journal.
    history_event_journal_ = std::make_unique<eft::HistoryEventJournal>();
    matched_gauss_dynamics_ =
        std::make_unique<eft::MatchedGaussDynamics>(lattice_size);
    // FTD-HISTORY-END
    colored_sites_cache_.reserve(256);
    // ARCH-2-M: backend_ is the single source of truth for backend selection.
    // Revision 3.1: the selection POLICY (GPU-default when CUDA is compiled
    // in) moved to make_default_backend() in backend.cpp — the last
    // backend-selection ifdef lives there, not here.
    backend_ = make_default_backend(*this, lattice_size);
}

// Destructor must be in .cpp where GpuEngine is fully defined (unique_ptr needs it)
RenderBridge::~RenderBridge() = default;

// Observation-only per-knot telemetry accessors. Out-of-line because
// KnotTracker is forward-declared in render_bridge.h (PIMPL; the type is only
// complete in this TU via the knot_telemetry.h include). Reading-only ⇒ these
// (and the gated record() call in tick()) are golden-hash neutral.
const KnotTracker& RenderBridge::knot_tracker() const { return *knot_tracker_; }
void RenderBridge::reset_knot_tracker() { knot_tracker_->clear(); }

// FTD-HISTORY-BEGIN: observation-only native event journal.
bool RenderBridge::enable_history_journal(bool enabled) {
    if (enabled && backend_ && backend_->kind() != Backend::Kind::Cpu) return false;
    history_event_journal_->set_enabled(enabled);
    return true;
}

bool RenderBridge::history_journal_enabled() const {
    return history_event_journal_->enabled();
}

void RenderBridge::clear_history_events() { history_event_journal_->clear(); }

std::vector<eft::HistoryEvent> RenderBridge::history_events() const {
    return history_event_journal_->snapshot();
}

std::uint64_t RenderBridge::rng_state_hash() const {
    return rng_state_->state_hash();
}

void RenderBridge::record_history_event(const eft::HistoryEvent& event) {
    history_event_journal_->record(event);
}

std::vector<int> RenderBridge::matched_state_snapshot() const {
    std::vector<int> state(voxels_.size(), 0);
    for (std::size_t i = 0; i < voxels_.size(); ++i)
        state[i] = static_cast<int>(voxels_[i].state);
    return state;
}

void RenderBridge::sync_matched_gauss_to_voxels() {
    if (!matched_gauss_dynamics_->initialized()) return;
    const int L = lattice_.size();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int i = lattice_.index(x, y, z);
                Voxel& voxel = voxels_[static_cast<std::size_t>(i)];
                voxel.flux = matched_gauss_dynamics_->centered_electric_at(x, y, z);
                voxel.flux_L = {};
                voxel.flux_R = {};
                voxel.wave_vel = {};
                voxel.wave_vel_L = {};
                voxel.wave_vel_R = {};
                delta_j_[static_cast<std::size_t>(i)] = {};
                delta_j_L_[static_cast<std::size_t>(i)] = {};
                delta_j_R_[static_cast<std::size_t>(i)] = {};
                dJ_[static_cast<std::size_t>(i)] = {};
            }
        }
    }
    mark_fields_dirty_from_voxels();
}

eft::MatchedMinimumEnergyResult
RenderBridge::initialize_matched_gauss_dynamics(double tolerance,
                                                 int max_iterations) {
#ifdef FTD_ENABLE_CUDA
    if (backend_ && backend_->kind() == Backend::Kind::Gpu && gpu_) {
        backend_->sync_to_host();
    }
#endif
    sync_ternary_from_voxels_if_needed();
    const auto result = matched_gauss_dynamics_->initialize_minimum_energy(
        matched_state_snapshot(), tolerance, max_iterations);
    if (result.valid) {
        sync_matched_gauss_to_voxels();
#ifdef FTD_ENABLE_CUDA
        if (backend_ && backend_->kind() == Backend::Kind::Gpu && gpu_) {
            gpu_->upload_matched_gauss(*matched_gauss_dynamics_);
        }
#endif
    }
    return result;
}

bool RenderBridge::matched_gauss_initialized() const {
    return matched_gauss_dynamics_->initialized();
}

const eft::MatchedGaussDynamics& RenderBridge::matched_gauss_state() const {
    return *matched_gauss_dynamics_;
}

bool RenderBridge::inject_matched_transverse_edge_potential(
    int x, int y, int z, int axis, double amplitude) {
    const bool applied = matched_gauss_dynamics_->inject_transverse_edge_potential(
        x, y, z, axis, amplitude);
    if (applied) {
        sync_matched_gauss_to_voxels();
#ifdef FTD_ENABLE_CUDA
        if (backend_ && backend_->kind() == Backend::Kind::Gpu && gpu_) {
            gpu_->upload_matched_gauss(*matched_gauss_dynamics_);
        }
#endif
    }
    return applied;
}

double RenderBridge::matched_gauss_voxel_sync_residual() const {
    if (!matched_gauss_dynamics_->initialized())
        return std::numeric_limits<double>::infinity();
    double residual = 0.0;
    const int L = lattice_.size();
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int i = lattice_.index(x, y, z);
                const Vec3 expected =
                    matched_gauss_dynamics_->centered_electric_at(x, y, z);
                const Vec3 actual = voxels_[static_cast<std::size_t>(i)].flux;
                residual = std::max(residual, std::abs(actual.x - expected.x));
                residual = std::max(residual, std::abs(actual.y - expected.y));
                residual = std::max(residual, std::abs(actual.z - expected.z));
            }
    return residual;
}
// FTD-HISTORY-END

void RenderBridge::sync_ternary_from_voxels() const {
    engine_state_.ternary.rebuild_from_voxels(voxels_);
    ternary_dirty_from_voxels_ = false;
}

void RenderBridge::sync_ternary_from_voxels_if_needed() const {
    if (!ternary_dirty_from_voxels_) return;
#if defined(_OPENMP)
    if (omp_in_parallel()) {
#pragma omp critical(ftd_render_bridge_ternary_sync)
        {
            if (ternary_dirty_from_voxels_) {
                sync_ternary_from_voxels();
            }
        }
        return;
    }
#elif defined(FTD_WASM_THREADS)
    ftd::with_critical([&] {
        if (ternary_dirty_from_voxels_) {
            sync_ternary_from_voxels();
        }
    });
    return;
#endif
    sync_ternary_from_voxels();
}

void RenderBridge::sync_fields_from_voxels() const {
    engine_state_.fields.rebuild_primary_from_voxels(voxels_);
    fields_dirty_from_voxels_ = false;
}

void RenderBridge::sync_fields_from_voxels_if_needed() const {
    if (!fields_dirty_from_voxels_) return;
#if defined(_OPENMP)
    if (omp_in_parallel()) {
#pragma omp critical(ftd_render_bridge_field_sync)
        {
            if (fields_dirty_from_voxels_) {
                sync_fields_from_voxels();
            }
        }
        return;
    }
#elif defined(FTD_WASM_THREADS)
    ftd::with_critical([&] {
        if (fields_dirty_from_voxels_) {
            sync_fields_from_voxels();
        }
    });
    return;
#endif
    sync_fields_from_voxels();
}

void RenderBridge::mark_fields_dirty_from_voxels() const {
    fields_dirty_from_voxels_ = true;
}

int8_t RenderBridge::set_state_unlocked(int idx, int8_t state) {
    const bool had_legacy_dirty = ternary_dirty_from_voxels_;
    if (ternary_dirty_from_voxels_) {
        sync_ternary_from_voxels();
    }
    const int8_t s = engine_state_.ternary.set_state(
        static_cast<std::size_t>(idx), state);
    voxels_[idx].state = s;
    ternary_dirty_from_voxels_ = had_legacy_dirty;
    return s;
}

void RenderBridge::set_state(int idx, int8_t state) {
#if defined(_OPENMP)
    if (omp_in_parallel()) {
#pragma omp critical(ftd_render_bridge_set_state)
        {
            set_state_unlocked(idx, state);
        }
        return;
    }
#elif defined(FTD_WASM_THREADS)
    ftd::with_critical([&] { set_state_unlocked(idx, state); });
    return;
#endif
    set_state_unlocked(idx, state);
}

int8_t RenderBridge::state_at(int idx) const {
    sync_ternary_from_voxels_if_needed();
    return engine_state_.ternary.state_at(static_cast<std::size_t>(idx));
}

bool RenderBridge::is_manifested(int idx) const {
    sync_ternary_from_voxels_if_needed();
    return engine_state_.ternary.is_manifested(static_cast<std::size_t>(idx));
}

long long RenderBridge::charge_sum() const {
    assert_sim_thread();
    sync_ternary_from_voxels_if_needed();
    return engine_state_.ternary.charge_sum();
}

const std::vector<int>& RenderBridge::active_indices() const {
    sync_ternary_from_voxels_if_needed();
    return engine_state_.ternary.active_indices();
}

const std::vector<int>& RenderBridge::ordered_active_indices() const {
    sync_ternary_from_voxels_if_needed();
    return engine_state_.ternary.ordered_active_indices();
}

const TernaryField& RenderBridge::ternary_field() const {
    sync_ternary_from_voxels_if_needed();
    return engine_state_.ternary;
}

const FieldSoA& RenderBridge::fields() const {
    if (backend_) backend_->sync_to_host();
    sync_fields_from_voxels_if_needed();
    return engine_state_.fields;
}

void RenderBridge::copy_visual_states(std::vector<std::int8_t>& out) {
  if (backend_) {
    // Scenario setup can leave direct host-side voxel edits pending before the
    // first tick. Make those edits visible to the compact GPU readback without
    // forcing a full device-to-host synchronization.
    backend_->flush_host_mutations();
    if (backend_->copy_visual_states(out)) return;
  }
  const auto& source = std::as_const(*this).voxels();
  out.resize(source.size());
  for (std::size_t i = 0; i < source.size(); ++i) out[i] = source[i].state;
}

void RenderBridge::copy_visual_flux_magnitude(std::vector<float>& out) {
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_visual_flux_magnitude(out)) return;
  }
  const auto& source = std::as_const(*this).voxels();
  out.resize(source.size());
  for (std::size_t i = 0; i < source.size(); ++i)
    out[i] = static_cast<float>(source[i].density());
}

void RenderBridge::copy_visual_flux_magnitude_plane(
    int axis, int index, std::vector<float>& out) {
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_visual_flux_magnitude_plane(axis, index, out)) return;
  }
  const auto& source = std::as_const(*this).voxels();
  const int L = lattice_.size();
  axis = axis == 0 ? 0 : (axis == 1 ? 1 : 2);
  index %= L;
  if (index < 0) index += L;
  out.resize(static_cast<std::size_t>(L) * L);
  for (int a = 0; a < L; ++a) {
    for (int b = 0; b < L; ++b) {
      int x, y, z;
      if (axis == 0)      { x = index; y = a; z = b; }
      else if (axis == 1) { x = a; y = index; z = b; }
      else                { x = a; y = b; z = index; }
      out[static_cast<std::size_t>(a) * L + b] =
          static_cast<float>(source[static_cast<std::size_t>(
              lattice_.index(x, y, z))].density());
    }
  }
}

void RenderBridge::copy_visual_particle_attributes(
    const std::vector<int>& indices, std::vector<float>& out) {
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_visual_particle_attributes(indices, out)) return;
  }
  const auto& source = std::as_const(*this).voxels();
  out.resize(indices.size() * 5u);
  for (std::size_t i = 0; i < indices.size(); ++i) {
    const int idx = indices[i];
    if (idx < 0 || static_cast<std::size_t>(idx) >= source.size()) continue;
    const auto& v = source[static_cast<std::size_t>(idx)];
    out[i * 5u + 0u] = static_cast<float>(v.remainder.x);
    out[i * 5u + 1u] = static_cast<float>(v.remainder.y);
    out[i * 5u + 2u] = static_cast<float>(v.remainder.z);
    out[i * 5u + 3u] = static_cast<float>(v.spin);
    out[i * 5u + 4u] = static_cast<float>(v.color);
  }
}

bool RenderBridge::begin_visual_snapshot(const VisualSnapshotRequest& request) {
  return backend_ && backend_->begin_visual_snapshot(request);
}

bool RenderBridge::visual_snapshot_ready() const {
  return backend_ && backend_->visual_snapshot_ready();
}

bool RenderBridge::poll_visual_snapshot(VisualSnapshot& out) {
  return backend_ && backend_->poll_visual_snapshot(out);
}

bool RenderBridge::visual_snapshot_safe_to_replace() const {
  return !backend_ || backend_->visual_snapshot_safe_to_replace();
}

bool RenderBridge::visual_snapshot_in_flight() const {
  return backend_ && backend_->visual_snapshot_in_flight();
}

Vec3 RenderBridge::flux_at(int idx) const {
    return fields().flux_at(static_cast<std::size_t>(idx));
}

Vec3 RenderBridge::wave_vel_at(int idx) const {
    return fields().wave_vel_at(static_cast<std::size_t>(idx));
}

double RenderBridge::density_at(int idx) const {
    return fields().density_at(static_cast<std::size_t>(idx));
}

GravityMetricAgg RenderBridge::gravity_metric_agg() const {
    assert_sim_thread();
    GravityMetricAgg a;
    if (backend_) {
        backend_->flush_host_mutations();
        if (backend_->copy_compact_gravity_metric(a)) return a;
    }
    const auto& vox = voxels();
    const int N = static_cast<int>(vox.size());
    double lat_sum = 0.0;
    int count = 0;
    for (int i = 0; i < N; ++i) {
        const double L = vox[i].latency;
        if (L <= 0.0) continue;
        if (L > a.latency_max) a.latency_max = L;
        const double g = vox[i].gamma_ftd();
        if (g > a.gamma_max) a.gamma_max = g;
        lat_sum += L;
        ++count;
    }
    a.voxel_count = count;
    if (count > 0) {
        a.latency_mean = lat_sum / count;
        a.f_min = 1.0 - a.latency_max * a.latency_max;
        a.dilation_max_pct = (1.0 - std::sqrt(std::max(0.0, a.f_min))) * 100.0;
    }
    // Keep `requested` distinct from `active`: a valid gravity profile can be
    // enabled before its first latency solve has produced any nonzero cells.
    // Both CPU and CUDA now source field_energy_gravity from
    // 1/2(|J|^2+|wave_vel|^2); backend parity is pinned by
    // gpu_native_extension_parity.
    a.requested = (toggles.latency_field || toggles.field_energy_gravity);
    a.active = a.requested && count > 0;
    return a;
}

// ARCH-4 (2026-04-25): propagates the seed to all RNG sources at once.
// Body lives here because GpuEngine is forward-declared in render_bridge.h.
void RenderBridge::seed_rng(unsigned int seed) {
    rng_state_->seed(seed);
    langevin_seed_initialized_ = false;   // force thread_seeds_ rederive
    toggles.langevin_seed       = seed;   // GPU cuRAND picks this up next tick
    // Revision 3.1: Backend virtual replaces the ifdef (CPU no-op).
    if (backend_) backend_->set_rng_seed(seed);
}

void RenderBridge::set_dt(double dt) {
    // E1 (FTD-0337): the Verlet wave integrator honors dt < 1 exactly like
    // the symplectic-leapfrog path (the FTD-0337 recon showed the default
    // non-symplectic leapfrog silently clamps dt to 1 — the "dt-invariance"
    // artifact). Both toggles default OFF ⇒ default behavior unchanged.
    if (toggles.lorentz_period2_floquet
        || toggles.lorentz_bcc_time_floquet) {
        // FTD-0408/0411 exact monodromies use the unit-step default kick-drift map.
        // Do not let physical_time() advertise a different tick duration from
        // the update whose pole was proved.
        dt_ = 1.0;
    } else {
        dt_ = (toggles.symplectic_leapfrog
               || toggles.verlet_wave_integrator
               || dt >= 1.0) ? dt : 1.0;
    }
    // ARCH-2: backend dispatch replaces the explicit ifdef. CpuBackend is a
    // no-op (RenderBridge::dt_ is the source of truth); GpuBackend forwards
    // to GpuEngine.
    if (backend_) backend_->set_dt(dt_);
}

// ARCH-2-J (2026-04-25): the private GPU sync delegators
// (gpu_sync_to_host, gpu_push_to_device, gpu_flush_host_mutations) were
// DELETED. All callers route through backend_->sync_to_host() /
// push_to_device() / flush_host_mutations() directly. Implementations live
// in src/backend.cpp::GpuBackend (friend access for state).

// Wave 5 (2026-04-14): GPU-aware phi_latency accessor.
// When use_gpu_ is true, lazily fetches the latency Poisson potential
// from the GPU buffer (d_phi_latency). When use_gpu_ is false, returns
// the CPU SOR solver's cached vector directly.
const std::vector<double>& RenderBridge::phi_latency() const {
    // ARCH-2-F: lazy fetch via Backend dispatch. CpuBackend is a no-op
    // (the SOR solver already writes phi_latency_ directly); GpuBackend
    // mirrors the device buffer into the host vector.
    if (backend_) {
        const_cast<Backend*>(backend_.get())->mirror_phi_latency();
    }
    return phi_latency_;
}

// ============================================================================
// Discrete operators — all moved to include/ftd/field_operators.h (R6, 2026-04-18)
// as inline free helpers. The public RenderBridge methods are inline forwarders
// defined in render_bridge.h. A handful of non-inline helpers remain here:
//   - sync_observable() (phase glue, mutates state)
//   - create_entangled_pair() (forwards to injection.cpp)
//   - compute_entropy() (forwards to diagnostics_compute.cpp)
// ============================================================================

void RenderBridge::sync_observable() {
  const int N = static_cast<int>(lattice_.total_sites());
  for (int i = 0; i < N; ++i)
    voxels_[i].flux = voxels_[i].flux_L + voxels_[i].flux_R;
  mark_fields_dirty_from_voxels();
}

void RenderBridge::create_entangled_pair(int x, int y, int z, const Vec3& flux_val) {
  ::ftd::create_entangled_pair_cpu(*this, x, y, z, flux_val);
}

double RenderBridge::compute_entropy() const { return ::ftd::compute_entropy_cpu(*this); }

// ============================================================================
// RULE 1: phase_read() — Wave propagation + state-flux coupling
//
// From the action principle δS/δJ = 0:
//   Wave: d²J/dt² = c²∇²J (Laplacian drives flux wave propagation)
//   Source: −g_c·∇(s) (manifested particles source OUTWARD flux, div J
//           toward the Gauss target; Term 2 sign amendment 2026-07-18)
//   Biot-Savart: g_c·∇×(s·v) (moving charges create rotational flux)
//
// STENCIL AND INTEGRATION NOTES (2026 audit):
//
//   18-point Moore Laplacian (weights face=1/3, edge=1/6, self=−4):
//   CONSISTENT (weights sum = 0) AND isotropic through O(h⁴). Direct
//   Taylor expansion:
//     face sum · (1/3)  +  edge sum · (1/6)  −  4·f
//       = h² ∇²f + (h⁴/12)·(∇²)²f + O(h⁶)
//   The 2:1 face:edge ratio is WHAT PRODUCES the O(h⁴) isotropy.
//   Verified empirically by tests/test_moore_laplacian_isotropy.cpp
//   (TRACKER §1.8 — closed 2026-04-17): smooth-Gaussian radial-symmetry
//   within 11% at L=64, σ=4. Residual at finite h is lattice dispersion
//   at k·h ~ 1 — a known artefact of ALL cubic-lattice FD schemes, not
//   a defect of these specific weights.
//
//   The advance pair (wave_vel += delta_J; flux += wave_vel) is
//   Störmer–Verlet (leapfrog) under the stagger interpretation where
//   wave_vel = v(t + h/2) and flux = J(t):
//       v(t + h/2) = v(t − h/2) + a(J(t)) · h   (kick)
//       J(t + h)   = J(t)       + v(t + h/2)·h   (drift)
//   Empirically verified by tests/test_leapfrog_integrator_audit.cpp
//   (see TRACKER_OPEN_ITEMS §1.4 — closed 2026-04-17): over 5000 ticks
//   with damping off, cumulative injection/dissipation balance to 0.1%,
//   the hallmark of a symplectic scheme.
//   C_SPEED = 1/√3 is a [SELECTION], NOT a forced CFL limit (FTD-0407).
//   The production normalised 18-point stencil has exact max symbol 16/3 at
//   (π,π,0), so stability allows c² ≤ 4/(16/3) = 3/4, i.e. c ≤ 0.866.
//   1/√3 is the CFL limit of the UNNORMALISED 6-point stencil, which is not
//   what the engine runs; it is conservative but unforced.
// ============================================================================

// Phase 4c (2026-04-27): phase_read decomposed into a single free function
// living in src/render_bridge_phases/phase_read.cpp. This method is now a
// thin orchestrator. The original parallel-for body (dual + single-substrate
// 18-pt isotropic Laplacian with interior fast path + boundary slow path,
// plus state-flux coupling) is preserved BYTE-IDENTICAL inside
// phase_read_main_loop. The bit-exact gate is test_render_bridge_golden
// (current pin: GOLDEN_HASH in test_render_bridge_golden.cpp).
void RenderBridge::phase_read() {
  sync_ternary_from_voxels_if_needed();
  ::ftd::phase_read_main_loop(*this);
}

// ============================================================================
// RULE 2: phase_write() — Commit flux, damping, manifestation/evaporation
//
// Flux update via leapfrog integration:
//   wave_vel += delta_J    (acceleration from Laplacian + source)
//   flux += wave_vel       (position update)
//   flux *= (1 - γ)        (dissipation, γ = α from ontic chain)
//
// Manifestation: when |J| > K_GENESIS at a void site, a particle manifests.
//   Polarity from sign(∇·J): sources → +1, sinks → -1
//   Spin from curl(J): local vorticity → ℤ₂ handedness
//   Color from dominant flux axis: 3 spatial dims → ℤ₃
//
// Evaporation: when |J| << K_B, particle returns to void.
// ============================================================================

// Phase 4a (2026-04-27): phase_write decomposed into 4 free functions
// living in src/render_bridge_phases/phase_write.cpp. This method is now
// a thin orchestrator. The bit-exact gate is test_render_bridge_golden.
// The free-function bodies are byte-identical with the original code
// (RF-4 dedup of the manifest body is the only structural change; the
// state/spin/color assignments themselves are unchanged).
void RenderBridge::phase_write() {
  // ARCH-7b (2026-04-25): pre-write flux snapshot for genesis curl reads.
  // Without it, sibling-thread voxel.flux writes race the curl reads —
  // making multi-thread runs non-deterministic. Cost: one O(N) copy.
  if (toggles.genesis) {
    snapshot_flux_pre_write(*this);
  }

  // Phase D: Precompute near-particle mask (O(N), race-free).
  if (toggles.selective_damping) {
    compute_near_particle_mask(*this);
  }

  // Main parallel-for: leapfrog (dual or single) + damping/Langevin OU
  // + genesis + evaporation. Per-thread RNG seeding handled internally.
  phase_write_main_loop(*this);

  // Sequential post-pass: convert pending particle_id sentinels (-2)
  // into deterministic IDs assigned in voxel-index order. ARCH-7 closes
  // TEST-004: replaying the same seed with any thread count produces
  // identical IDs.
  phase_write_assign_pending_ids(*this);
}

// ============================================================================
// RULE 3: gauss_project() — Enforce ∇·J = s (charge conservation)
//
// This is the U(1) gauge constraint. It is logically necessary:
// charge conservation demands that the divergence of the flux field
// equals the charge density at every point.
//
// Method: SOR (Successive Over-Relaxation) on ∇²φ = ∇·J − s,
// then correct J -= ∇φ to remove unphysical longitudinal modes.
//
// Warm-started: phi_ persists between ticks for fast reconvergence.
// SOR ω=1.75 matches the Coulomb solver quality.
// ============================================================================

// ============================================================================
// Poisson solvers — core bodies extracted to poisson_solvers.cpp (R1, 2026-04-18).
// RenderBridge keeps ownership of phi_/phi_coulomb_/phi_latency_/sor_source_
// buffers; the methods below are thin wrappers over the free functions.
// ============================================================================

// Thin wrappers delegating to poisson_solvers.cpp.
void RenderBridge::gauss_project() {
  gauss_project_cpu(voxels_, ternary_field(), phi_, sor_source_, lattice_, toggles.dual_substrate,
                    toggles.exact_dual_gauss, toggles.coulomb_charge_coupling, sor_iterations_);
}

void RenderBridge::solve_coulomb_poisson() {
  solve_coulomb_poisson_cpu(ternary_field(), phi_coulomb_, sor_source_, lattice_, sor_iterations_,
                            toggles.coulomb_source_scale);
}

void RenderBridge::solve_latency_poisson() {
  const std::vector<StrongStressCell>* strong_cells = nullptr;
  if (toggles.strong_stress_energy) {
    compute_strong_stress_cells(*this, strong_stress_cells_);
    strong_cells = &strong_stress_cells_;
  }
  solve_latency_poisson_cpu(voxels_, ternary_field(), phi_latency_, sor_source_, lattice_, sor_iterations_,
                            toggles.field_energy_gravity, strong_cells);
}

// ============================================================================
// RULE 4: phase_forces() — Field-mediated forces ONLY
//
// Coulomb force via Poisson-solved potential (Phase 3):
//   Solve ∇²φ_C = s (charge density) via warm-started SOR
//   F_EM = -α · s · ∇φ_C          (proper 1/r² from Poisson Green's function)
//
// Legacy mode (poisson_coulomb = false):
//   F_EM = -α · s · ∇(∇·J)        (local double gradient — r^(-3.8) falloff)
//
// From density gradient (gravitational attraction to flux concentrations):
//   F_grav = G_N · ∇ρ
//
// Pairwise colour / Yukawa / exchange sit in the same loop (default OFF).
// ============================================================================

void RenderBridge::phase_forces() {
  // Phase 4b (2026-04-27): decomposed into three free functions in
  // engine/src/render_bridge_phases/phase_forces.cpp. The body that lived
  // here (~225 LOC of EM/gravity/Lorentz/color force computation plus
  // γ_FTD relativistic momentum integration) is preserved BYTE-IDENTICAL
  // inside phase_forces_main_loop. The golden-tick test
  // (test_render_bridge_golden; current pin: GOLDEN_HASH in that file) is
  // the strict gate on this refactor.
  ::ftd::phase_forces_solve_potentials(*this);
  ::ftd::phase_forces_build_color_cache(*this);
  ::ftd::phase_forces_main_loop(*this);
  // Phase 2 (unified mass): rigid-body cluster inertia. Default-off, additive —
  // the per-voxel loop already skips locked voxels, so when the toggle is off
  // this is a no-op and the golden hash is byte-identical.
  if (toggles.cluster_inertia) ::ftd::phase_forces_integrate_clusters(*this);
}


// ============================================================================
// RULE 5: phase_movement() — Kinematics + collisions + annihilation
//
// Particles move on the lattice via remainder accumulation:
//   remainder += velocity
//   when |remainder| >= 1 on any axis → integer lattice jump
//
// Collision outcomes (logically determined):
//   - Target void: move into it, carry self-field
//   - Target same sign: elastic bounce (two things can't be in one place)
//   - Target opposite sign: annihilation (cancel to void, flux burst)
// ============================================================================

// Phase 4c (2026-04-27): phase_movement decomposed into a single free
// function living in src/render_bridge_phases/phase_movement.cpp. This method
// is now a thin orchestrator. The original sequential per-voxel loop body
// (drift + integer-jump dispatch + void-target move with self-field carry +
// same-sign elastic bounce + opposite-sign annihilation with 6-neighbor flux
// burst, dual-substrate-aware) is preserved BYTE-IDENTICAL inside
// phase_movement_main_loop. The bit-exact gate is test_render_bridge_golden
// (current pin: GOLDEN_HASH in that file). Splitting the per-voxel body further would break
// the golden gate — see the header comment for why.
void RenderBridge::phase_movement() {
  ::ftd::phase_movement_main_loop(*this);
}

// ============================================================================
// The Tick: Six rules, executed in order
// ============================================================================

void RenderBridge::tick() {
  assert_sim_thread();
  causal_projection_events_this_tick_ = 0;
  // FTD-HISTORY-BEGIN: observation-only native event journal.
  if (history_event_journal_->enabled()) history_event_journal_->clear();
  // FTD-HISTORY-END

  // A caller can leave dt_<1 behind by enabling an alternate integrator,
  // calling set_dt(), then switching toggles. FTD-0408/0411 are defined only
  // for the unit-step default map, so normalize stale state before validation
  // and before physical_time_ advances.
  if ((toggles.lorentz_period2_floquet
       || toggles.lorentz_bcc_time_floquet) && dt_ != 1.0) {
    dt_ = 1.0;
    if (backend_) backend_->set_dt(dt_);
  }
  // F3 (callstack audit 2026-04-17): validate runs on BOTH paths now
  // so toggle-combination warnings surface regardless of CPU/GPU build.
  //
  // ARCH-3 (2026-04-25): throw under strict_validation; otherwise emit ONE
  // warning per unique error string per bridge instance (last_validation_warn_)
  // so tests don't spam stderr every tick with the same message.
  {
      std::string validErr;
      if (!toggles.validate(&validErr)) {
          if (toggles.strict_validation || toggles.matched_gauss_dynamics) {
#ifdef __EMSCRIPTEN__
              // WASM build compiles with -fno-exceptions; downgrade to
              // stderr + abort so strict_validation still surfaces
              // configuration bugs instead of silently passing.
              std::cerr << "[TermToggles] FATAL invalid combination: " << validErr << std::endl;
              std::abort();
#else
              throw std::logic_error("[TermToggles] Invalid combination: " + validErr);
#endif
          }
          if (validErr != last_validation_warn_) {
              std::cerr << "[TermToggles] Invalid combination: " << validErr;
              last_validation_warn_ = validErr;
          }
      } else if (!last_validation_warn_.empty()) {
          // Reset memo when toggles get fixed mid-run.
          last_validation_warn_.clear();
      }
  }

  if (toggles.matched_gauss_dynamics && dt_ != 1.0) {
#ifdef __EMSCRIPTEN__
    std::cerr << "[FTD-0428] FATAL: matched_gauss_dynamics requires dt=1\n";
    std::abort();
#else
    throw std::logic_error(
        "[FTD-0428] matched_gauss_dynamics requires the locked unit tick");
#endif
  }

  sync_ternary_from_voxels_if_needed();

  // ARCH-2-D: GPU dispatch via Backend. CpuBackend::tick() falls through to
  // the CPU phase ladder below; GpuBackend::tick() owns the full flush →
  // engine->tick → sync_to_host sequence.
  if (backend_ && backend_->kind() == Backend::Kind::Gpu) {
    backend_->tick();
    // Observation-only knot telemetry on the GPU path: record() reads
    // voxels()/current_tick(), and the voxels() accessor syncs device→host
    // first, so it sees settled state. Golden-neutral (read-only).
    if (toggles.knot_tracking) knot_tracker_->record(*this);
    // Rule 8 (tau and optional de-Broglie phase) is device-resident on CUDA.
    // Do not materialize and re-upload the full lattice here.
#ifdef FTD_ENABLE_CUDA
    if (interactive_gpu_mode_ && gpu_dirty_) {
      // The host AoS shadow (voxels_) is deliberately stale on the interactive
      // GPU path — the 469 B/site device->host mirror is deferred every tick.
      // Summing that stale shadow via update_energy_ledger_cpu would report
      // E_curr = 0, which is exactly what the native app's energy readout and
      // telemetry chart showed. Instead source the ledger from the compact
      // device-side energy_audit() reduction (a few scalars D2H, NOT the full
      // field transfer). CPU and non-interactive GPU keep the host-shadow
      // formula below (byte-identical to prior behavior).
      update_energy_ledger_from_audit();
      return;
    }
#endif
    update_energy_ledger();
    return;
  }

  // F2 (callstack audit 2026-04-17): CPU-only warning for GPU-only toggles.
  // Printed once per RenderBridge instance on the first tick where such a
  // toggle is set, so it's discoverable but doesn't spam.
  if (!cpu_warnings_emitted_) {
    std::string gpu_only_msg = toggles.cpu_runtime_warnings();
    if (!gpu_only_msg.empty()) {
      std::cerr << "[TermToggles] CPU-build warning:\n" << gpu_only_msg;
      cpu_warnings_emitted_ = true;
    }
  }

  // Rule 1: Wave propagation + state-flux coupling
  // FTD-0271: the de Broglie clock's KG mass term -omega0^2*J is computed in
  // phase_read (it writes delta_j), so phase_read must run when the clock is on
  // even if the wave and coupling terms are both off (the pure k=0 rest-frame
  // clock: each manifested voxel oscillates at omega0 with no spatial term).
  //
  // FTD-0281: db_clock_coulomb is a CPU/RenderBridge spectroscopy diagnostic
  // for the FTD-0278 operator. It needs the live Coulomb potential before
  // phase_read so the diagonal KG term can read V(r) on the same tick. Forces
  // are validation-conflicted with this toggle, so this pre-read solve is the
  // only Coulomb solve in the v1 diagnostic phase order.
  // EW phase-transition background sweep: sinusoidal uniform +x flux drive.
  // D(tick) = (sin(tick_ * 0.01) + 1) / 2 * 0.05, advancing 0.01 rad/tick.
  // Runs before phase_read so the injected flux is processed by the wave
  // equation in the same cycle (matches the JS setInterval(16ms) rate at 60fps).
  // Dual mode: phase_read consumes flux_L/flux_R only and phase_write rebuilds
  // flux := L+R, so the drive must enter the registers to enter the dynamics
  // at all (census EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING §5.3, reconciled
  // 2026-07-17). Symmetric half/half split — a uniform background injects no
  // chirality — matching the WASM uniform-background injector
  // (bindings_render_bridge.cpp) and inject_flux_add_cpu.
  if (toggles.ew_background_sweep) {
      const double D = (std::sin(tick_ * 0.01) + 1.0) / 2.0 * 0.05;
      auto& vox = voxels();
      const int L = lattice().size();
      const bool dual = toggles.dual_substrate;
      for (int z = 0; z < L; ++z) for (int y = 0; y < L; ++y) for (int x = 0; x < L; ++x) {
          auto& v = vox[lattice().index(x, y, z)];
          v.flux.x += D;
          if (dual) {
              v.flux_L.x += D * 0.5;
              v.flux_R.x += D * 0.5;
          }
      }
  }

  if (toggles.db_clock_coulomb)
    solve_coulomb_poisson();

  if (toggles.wave_propagation || toggles.coupling || toggles.de_broglie_clock)
    phase_read();

  // Rule 2: Commit flux, damping, manifestation/evaporation
  if (!toggles.matched_gauss_dynamics) {
    phase_write();
  } else {
    genesis_events_this_tick_ = 0;
    evaporation_events_this_tick_ = 0;
  }

  // Rule 2a (E1, FTD-0333 §5.1 / FTD-0337): velocity-Verlet (KDK) completion.
  // phase_write applied half-kick + drift; here we recompute the acceleration
  // at the post-drift field (phase_read is deterministic and only writes the
  // delta_j buffers) and apply the second half-kick to wave_vel. The wave
  // sub-integrator is thereby KDK-complete BEFORE the Gauss projection — the
  // projection remains a separate constraint map applied between wave steps
  // (operator splitting; same interleaving as the legacy leapfrog). Genesis
  // state changes from phase_write are visible to the re-read via set_state's
  // synchronous ternary update, so the coupling source −g_c·∇s is current.
  // Default OFF ⇒ dead branch ⇒ golden hash 0xb604d81a3d79366e untouched.
  if (toggles.verlet_wave_integrator) {
    phase_read();
    const int Nv = static_cast<int>(lattice_.total_sites());
    const double half_dt = 0.5 * dt_;
    if (toggles.dual_substrate) {
      for (int i = 0; i < Nv; ++i) {
        Voxel& v = voxels_[i];
        v.wave_vel_L += delta_j_L_[i] * half_dt;
        v.wave_vel_R += delta_j_R_[i] * half_dt;
        v.wave_vel = v.wave_vel_L + v.wave_vel_R;
      }
    } else {
      for (int i = 0; i < Nv; ++i) {
        voxels_[i].wave_vel += delta_j_[i] * half_dt;
      }
    }
  }

  // Rule 2b: Pair production (correlated ±1 pairs from high-flux void).
  // F2 (callstack audit 2026-04-17): matching GPU path order. No-op on
  // CPU until the pair-production CPU port lands.
  if (toggles.pair_production)
    pair_production_cpu();

  // Rule 3: Gauss constraint enforcement (∇·J = s)
  if (toggles.gauss_projection)
    gauss_project();

  // Rule 3b: Self-field floor REMOVED (Phase 4 — Energy Conservation). The
  // former per-tick reset of self_field_injection_ was also a no-op — the
  // member is default-initialised 0 and nothing else writes to it now that
  // the floor is gone. (F1 from callstack audit 2026-04-17.)

  // FTD-0406: snapshot the selected strong Hamiltonian immediately before
  // latency/forces. Static movement-off configurations only build local T00;
  // the collision-free projection activates when movement is enabled.
  if (toggles.strong_stress_energy)
    begin_strong_energy_step(*this);

  // Rule 3c: Latency field (gravitational potential) — Poisson solver
  // ∇²φ_L = 4πG·ρ_mass, then L = √(clamp(φ_L, 0, 0.998))
  // Must run after Gauss (which modifies flux) and before forces (which use L).
  if (toggles.latency_field)
    solve_latency_poisson();

  // Rule 4: Field-mediated forces. EM/gravity/Lorentz stay inside
  // phase_forces_main_loop gated on toggles.forces; colour / Yukawa /
  // exchange run when their own toggles are on so a Yukawa-only tick
  // does not accidentally apply legacy EM.
  if (toggles.forces || toggles.color_forces || toggles.strong_force
      || toggles.exchange_force || toggles.cluster_inertia)
    phase_forces();

  // FTD-0428: snapshot the exact ternary source immediately before the only
  // permitted state-changing phase. The post-movement difference is routed
  // into oriented face current; reaction-bearing histories fail closed.
  std::vector<int> matched_before;
  if (toggles.matched_gauss_dynamics) {
    if (!matched_gauss_dynamics_->initialized()) {
#ifdef __EMSCRIPTEN__
      std::cerr << "[FTD-0428] FATAL: matched_gauss_dynamics requires explicit initialization\n";
      std::abort();
#else
      throw std::logic_error(
          "[FTD-0428] matched_gauss_dynamics requires explicit initialization");
#endif
    }
    matched_before = matched_state_snapshot();
  }

  // Rule 5: Movement + collisions + annihilation
  if (toggles.movement)
    phase_movement();

  if (toggles.matched_gauss_dynamics) {
    const auto matched_after = matched_state_snapshot();
    eft::DualCellContinuity history;
    const auto extraction = eft::extract_moore_history_from_snapshots(
        lattice_.size(), matched_before, matched_after, history);
    const auto step = extraction.valid
        ? matched_gauss_dynamics_->advance(history, C_SPEED, dt_, 1e-12)
        : eft::MatchedWaveStep{};
    if (!extraction.valid || !step.valid) {
#ifdef __EMSCRIPTEN__
      std::cerr << "[FTD-0428] FATAL: movement history is not conservative and routable\n";
      std::abort();
#else
      throw std::logic_error(
          "[FTD-0428] movement history is not conservative and routable");
#endif
    }
    sync_matched_gauss_to_voxels();
  }

  // FTD-0406: preserve the proposal positions and project only relative
  // physical momenta onto H_strong(t+dt)=H_strong(t). Topology changes are
  // surfaced through explicit diagnostics rather than hidden by bookkeeping.
  if (toggles.strong_stress_energy && toggles.movement)
    complete_strong_energy_step(*this);

  // Rule 5b: Absorbing-boundary sponge — disperse outgoing waves into the void
  // at the lattice faces. Runs AFTER gauss/forces/movement (the last flux
  // writers) so the damped edge shell is NOT refilled by the Gauss projection.
  // Gated; default off → golden-tick hash + conservation tests unchanged.
  if (toggles.absorbing_boundary)
    apply_absorbing_boundary(*this);

  // Rule 5c: flux-field boundary law. Periodic (toroidal wrap) is the default
  // and is handled by the lattice neighbour tables — no pass needed. The
  // Reflective / Dispersal passes re-impose their boundary on the shell AFTER
  // the last flux writers (same placement rationale as the sponge above).
  // Default Periodic → neither pass runs → golden-tick hash unchanged.
  if (toggles.flux_boundary == FluxBoundaryMode::Reflective)
    apply_reflective_flux_boundary(*this);
  else if (toggles.flux_boundary == FluxBoundaryMode::Dispersal)
    apply_dispersal_flux_boundary(*this);


  // Self-field floor moved to Rule 3b (after Gauss, before forces) in Phase 3.
  // No second floor here — eliminates the double-injection energy leak.

  // Rule 6: Weak transmutation (polarity flip under field stress).
  // F5 (callstack audit 2026-04-17): extracted to weak_transmutation_cpu().
  if (toggles.weak_transmutation)
    weak_transmutation_cpu();

  // Rule 7: Triad binding detection (3 same-sign particles → locked).
  // After movement + weak, matching GpuEngine::record_tick_body().
  if (toggles.triad_binding)
    triad_binding_cpu();

  // Rule 7b: non-Abelian gauge-link relaxation (revision 0.9 option a).
  // [IMPOSED] Wilson-action staple relaxation imported from standard lattice
  // gauge theory — one Jacobi sweep per tick over the SU(2)/SU(3) edge link
  // variables. The links are WRITE-ONLY w.r.t. the substrate: nothing
  // downstream consumes them (color_forces uses color labels, not links), so
  // this phase cannot alter voxel state, energy audit, or any golden hash —
  // enforced by test_gauge_links G1a. Buffers are lazily allocated inside the
  // relax calls (revision 4.1b). Default OFF ⇒ golden-neutral.
  if (toggles.su2_gauge)
    relax_su2_links_cpu(*this, GAUGE_RELAX_DT, GAUGE_RELAX_BETA);
  if (toggles.su3_gauge)
    relax_su3_links_cpu(*this, GAUGE_RELAX_DT, GAUGE_RELAX_BETA);

  // Rule 8: Proper time accumulation (gravity sector).
  // F5 (callstack audit 2026-04-17): extracted to accumulate_proper_time().
  // FTD-0271 (A5): also run when the de Broglie clock is on (latency_field may
  // be off) so the clock phase advances; at L=0 dτ=√(1−v²) gives the SR rate.
  if (toggles.latency_field || toggles.de_broglie_clock)
    accumulate_proper_time();

  physical_time_ += dt_;
  ++tick_;

  // Observation-only knot telemetry (golden-neutral; reads settled state only).
  if (toggles.knot_tracking) knot_tracker_->record(*this);

  sync_ternary_from_voxels_if_needed();
  mark_fields_dirty_from_voxels();

  // ── Conservation bookkeeping (fills EnergyLedger) ────────────────────
  // Cheap: a few adds + divides; no loop over N. Tests assert on
  // `energy_ledger().residual` rather than re-deriving totals.
  update_energy_ledger();
}

// ════════════════════════════════════════════════════════════════════════
// Transmutation phase bodies extracted to transmutation_phases.cpp
// (R2, 2026-04-18). The RenderBridge:: methods below stay as thin
// wrappers so tick() and existing callers keep working unchanged.
// ════════════════════════════════════════════════════════════════════════
// (Headers moved to top of file to avoid nested-namespace include.)

void RenderBridge::weak_transmutation_cpu() { ::ftd::weak_transmutation_cpu(*this); }
void RenderBridge::accumulate_proper_time() { ::ftd::accumulate_proper_time(*this); }
void RenderBridge::pair_production_cpu()    { ::ftd::pair_production_cpu(*this);    }
void RenderBridge::triad_binding_cpu()      { ::ftd::triad_binding_cpu(*this);      }

// Energy-ledger body extracted to energy_ledger_compute.cpp (R3, 2026-04-18).
void RenderBridge::update_energy_ledger() { ::ftd::update_energy_ledger_cpu(*this); }
void RenderBridge::update_energy_ledger_from_audit() {
  ::ftd::update_energy_ledger_from_audit(*this);
}

eft::DualCellContinuity RenderBridge::continuity_step() const {
  assert_sim_thread();
  // Revision 3.1 (was ARCH-2-K's ifdef): Backend virtual dispatch. The
  // GpuBackend override fills `out` from the device; CPU backends return
  // false and the host default is used. Semantics-preserving under
  // force_cpu(): gpu_engine_ptr() already gated on the ACTIVE backend.
  eft::DualCellContinuity out(lattice_.size());
  const_cast<RenderBridge*>(this)->backend_->continuity_step(out);
  return out;
}

void RenderBridge::run(int num_ticks) {
  // 2026-04-25 (engine-expert T4): the previous CUDA fast-path delegated to
  // gpu_->run(num_ticks) and called accumulate_proper_time/update_energy_ledger
  // exactly once at the end of the batch. This silently differed from the CPU
  // path (which loops tick() and computes the ledger every tick), so any test
  // asserting on energy_ledger().residual after run(N) only saw the final
  // tick on GPU, masking intermediate violations.
  //
  // Fix: always loop tick() so the ledger and proper-time accumulator are
  // identical on CPU and GPU, tick by tick. Non-interactive callers pay the
  // canonical host snapshot cost; native interactive mode remains resident.
  // Campaigns that need a batched device-only path may call GpuEngine::run.
  for (int i = 0; i < num_ticks; ++i) {
    tick();
  }
}

// ============================================================================
// Diagnostics / audits / EM decomposition — bodies in diagnostics_compute.cpp
// (R4, 2026-04-18). The methods below are thin wrappers.
// ============================================================================
Diagnostics RenderBridge::diagnostics() const {
  assert_sim_thread();
  Diagnostics d;
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_compact_diagnostics(d)) return d;
  }
  return ::ftd::compute_diagnostics(*this);
}

EnergyAudit RenderBridge::energy_audit() const {
  assert_sim_thread();
  EnergyAudit a;
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_compact_energy_audit(a)) {
      a.self_field_injection = self_field_injection_;
      return a;
    }
  }
  a = ::ftd::compute_energy_audit(*this);
  a.self_field_injection = self_field_injection_;  // private, stitched in here
  return a;
}

bool RenderBridge::begin_telemetry_snapshot(
    const TelemetrySnapshotRequest& request) {
  assert_sim_thread();
  return backend_ && backend_->begin_telemetry_snapshot(request);
}

bool RenderBridge::telemetry_snapshot_ready() const {
  return backend_ && backend_->telemetry_snapshot_ready();
}

bool RenderBridge::poll_telemetry_snapshot(TelemetrySnapshot& out) {
  return backend_ && backend_->poll_telemetry_snapshot(out);
}

bool RenderBridge::copy_compact_lagrangian(LagrangianDiag& out) const {
  assert_sim_thread();
  if (!backend_) return false;
  backend_->flush_host_mutations();
  return backend_->copy_compact_lagrangian(out);
}

VoxelInspection RenderBridge::inspect_voxel(int x, int y, int z) const {
  assert_sim_thread();
  VoxelInspection out;
  const int idx = lattice_.index(x, y, z);
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_compact_voxel(idx, out)) return out;
  }
  const auto& source = voxels();
  out.voxel = source[static_cast<std::size_t>(idx)];
  out.divergence = divergence_flux(idx);
  out.curl = curl_flux(idx);
  out.em.E = out.voxel.wave_vel * -1.0;
  out.em.B = out.curl;
  out.em.E_mag = out.em.E.mag();
  out.em.B_mag = out.em.B.mag();
  return out;
}

ForceDiag RenderBridge::inspect_force(int x, int y, int z) const {
  assert_sim_thread();
  ForceDiag out;
  const int idx = lattice_.index(x, y, z);
  if (backend_) {
    backend_->flush_host_mutations();
    if (backend_->copy_compact_force(idx, out)) return out;
  }
  // sync_to_host() also scatters the device force arrays into force_diag_.
  (void)voxels();
  return force_diag_[static_cast<std::size_t>(idx)];
}

const std::vector<StrongStressCell>& RenderBridge::strong_stress_cells() const {
  compute_strong_stress_cells(*this, strong_stress_cells_);
  return strong_stress_cells_;
}

EMFieldDiag RenderBridge::em_field_at(int idx) const { return ::ftd::compute_em_field_at(*this, idx); }
Vec3 RenderBridge::poynting_vector(int idx) const    { return ::ftd::compute_poynting_vector(*this, idx); }

// ============================================================================
// Injection + aggregate profile — bodies in injection.cpp (R5, 2026-04-18).
// ============================================================================
void RenderBridge::inject_flux(int x, int y, int z, const Vec3 &flux_val) {
  ::ftd::inject_flux_cpu(*this, x, y, z, flux_val);
}
void RenderBridge::inject_flux_add(int x, int y, int z, const Vec3 &flux_val) {
  ::ftd::inject_flux_add_cpu(*this, x, y, z, flux_val);
}
void RenderBridge::inject_wave_vel_add(int x, int y, int z, const Vec3 &wv_val) {
  ::ftd::inject_wave_vel_add_cpu(*this, x, y, z, wv_val);
}
void RenderBridge::inject_particle(int x, int y, int z, int8_t state,
                                   const Vec3 &flux_val, int8_t spin, int8_t color) {
  ::ftd::inject_particle_cpu(*this, x, y, z, state, flux_val, spin, color);
}
void RenderBridge::inject_wavepacket(int cx, int cy, int cz, int8_t state,
                                     double sigma, double amplitude) {
  ::ftd::inject_wavepacket_cpu(*this, cx, cy, cz, state, sigma, amplitude);
}
AggregateProfile RenderBridge::aggregate_profile(int center_idx, double threshold) const {
  return ::ftd::compute_aggregate_profile(*this, center_idx, threshold);
}

}  // namespace ftd
