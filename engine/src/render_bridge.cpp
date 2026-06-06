/**
 * Logic-First FTD Engine (v2.0)
 *
 * Built from axioms: {3D lattice, ternary states, flux field, local causality}
 *
 * Six rules, nothing else:
 *   1. Flux wave equation: d²J/dt² = c²∇²J (local linear dynamics)
 *   2. State-flux coupling: g_c·∇(s) source term (from δS/δJ = 0)
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
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

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
      sor_source_(lattice_.total_sites(), 0.0),
      su2_links_x_(lattice_.total_sites()),
      su2_links_y_(lattice_.total_sites()),
      su2_links_z_(lattice_.total_sites()),
      su3_links_x_(lattice_.total_sites()),
      su3_links_y_(lattice_.total_sites()),
      su3_links_z_(lattice_.total_sites())
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
    colored_sites_cache_.reserve(256);
#ifdef FTD_ENABLE_CUDA
    gpu_ = std::make_unique<gpu::GpuEngine>(lattice_size);
    // ARCH-2-M: backend_ is now the single source of truth for backend
    // selection (the legacy `use_gpu_` flag was deleted).
    backend_ = std::make_unique<GpuBackend>(*this, gpu_.get());
    std::cerr << "[RenderBridge] GPU backend active (CUDA, L=" << lattice_size << ")\n";
#else
    backend_ = std::make_unique<CpuBackend>(*this);
#endif
}

// Destructor must be in .cpp where GpuEngine is fully defined (unique_ptr needs it)
RenderBridge::~RenderBridge() = default;

void RenderBridge::sync_ternary_from_voxels() const {
    engine_state_.ternary.rebuild_from_voxels(voxels_);
    ternary_dirty_from_voxels_ = false;
}

void RenderBridge::sync_ternary_from_voxels_if_needed() const {
    if (!ternary_dirty_from_voxels_) return;
#ifdef _OPENMP
    if (omp_in_parallel()) {
#pragma omp critical(ftd_render_bridge_ternary_sync)
        {
            if (ternary_dirty_from_voxels_) {
                sync_ternary_from_voxels();
            }
        }
        return;
    }
#endif
    sync_ternary_from_voxels();
}

void RenderBridge::sync_fields_from_voxels() const {
    engine_state_.fields.rebuild_primary_from_voxels(voxels_);
    fields_dirty_from_voxels_ = false;
}

void RenderBridge::sync_fields_from_voxels_if_needed() const {
    if (!fields_dirty_from_voxels_) return;
#ifdef _OPENMP
    if (omp_in_parallel()) {
#pragma omp critical(ftd_render_bridge_field_sync)
        {
            if (fields_dirty_from_voxels_) {
                sync_fields_from_voxels();
            }
        }
        return;
    }
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
#ifdef _OPENMP
    if (omp_in_parallel()) {
#pragma omp critical(ftd_render_bridge_set_state)
        {
            set_state_unlocked(idx, state);
        }
        return;
    }
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
    GravityMetricAgg a;
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
    a.active = (toggles.latency_field || toggles.field_energy_gravity) && count > 0;
    return a;
}

// ARCH-4 (2026-04-25): propagates the seed to all RNG sources at once.
// Body lives here because GpuEngine is forward-declared in render_bridge.h.
void RenderBridge::seed_rng(unsigned int seed) {
    rng_state_->seed(seed);
    langevin_seed_initialized_ = false;   // force thread_seeds_ rederive
    toggles.langevin_seed       = seed;   // GPU cuRAND picks this up next tick
#ifdef FTD_ENABLE_CUDA
    if (gpu_) gpu_->set_rng_seed(seed);
#endif
}

void RenderBridge::set_dt(double dt) {
    dt_ = (toggles.symplectic_leapfrog || dt >= 1.0) ? dt : 1.0;
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
//   Source: g_c·∇(s) (manifested particles source flux in their neighborhood)
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
//   the hallmark of a symplectic scheme. C_SPEED = 1/√D = 1/√3 is the
//   leapfrog CFL limit, correctly identified.
// ============================================================================

// Phase 4c (2026-04-27): phase_read decomposed into a single free function
// living in src/render_bridge_phases/phase_read.cpp. This method is now a
// thin orchestrator. The original parallel-for body (dual + single-substrate
// 18-pt isotropic Laplacian with interior fast path + boundary slow path,
// plus state-flux coupling) is preserved BYTE-IDENTICAL inside
// phase_read_main_loop. The bit-exact gate is test_render_bridge_golden
// (hash 0xcd957b601d47868a).
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
  solve_coulomb_poisson_cpu(ternary_field(), phi_coulomb_, sor_source_, lattice_, sor_iterations_);
}

void RenderBridge::solve_latency_poisson() {
  solve_latency_poisson_cpu(voxels_, ternary_field(), phi_latency_, sor_source_, lattice_, sor_iterations_,
                            toggles.field_energy_gravity);
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
// NO pairwise forces. NO Yukawa. NO exchange. NO QCD running.
// ============================================================================

void RenderBridge::phase_forces() {
  // Phase 4b (2026-04-27): decomposed into three free functions in
  // engine/src/render_bridge_phases/phase_forces.cpp. The body that lived
  // here (~225 LOC of EM/gravity/Lorentz/color force computation plus
  // γ_FTD relativistic momentum integration) is preserved BYTE-IDENTICAL
  // inside phase_forces_main_loop. The golden-tick test
  // (test_render_bridge_golden, hash 0xcd957b601d47868a) is the strict
  // gate on this refactor.
  ::ftd::phase_forces_solve_potentials(*this);
  ::ftd::phase_forces_build_color_cache(*this);
  ::ftd::phase_forces_main_loop(*this);
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
// (hash 0xcd957b601d47868a). Splitting the per-voxel body further would break
// the golden gate — see the header comment for why.
void RenderBridge::phase_movement() {
  ::ftd::phase_movement_main_loop(*this);
}

// ============================================================================
// The Tick: Six rules, executed in order
// ============================================================================

void RenderBridge::tick() {
  // F3 (callstack audit 2026-04-17): validate runs on BOTH paths now
  // so toggle-combination warnings surface regardless of CPU/GPU build.
  //
  // ARCH-3 (2026-04-25): throw under strict_validation; otherwise emit ONE
  // warning per unique error string per bridge instance (last_validation_warn_)
  // so tests don't spam stderr every tick with the same message.
  {
      std::string validErr;
      if (!toggles.validate(&validErr)) {
          if (toggles.strict_validation) {
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

  sync_ternary_from_voxels_if_needed();

  // ARCH-2-D: GPU dispatch via Backend. CpuBackend::tick() falls through to
  // the CPU phase ladder below; GpuBackend::tick() owns the full flush →
  // engine->tick → sync_to_host sequence.
  if (backend_ && backend_->kind() == Backend::Kind::Gpu) {
    backend_->tick();
    if (toggles.latency_field)
      accumulate_proper_time();
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
  if (toggles.wave_propagation || toggles.coupling)
    phase_read();

  // Rule 2: Commit flux, damping, manifestation/evaporation
  phase_write();

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

  // Rule 3c: Latency field (gravitational potential) — Poisson solver
  // ∇²φ_L = 4πG·ρ_mass, then L = √(clamp(φ_L, 0, 0.998))
  // Must run after Gauss (which modifies flux) and before forces (which use L).
  if (toggles.latency_field)
    solve_latency_poisson();

  // Rule 4: Field-mediated forces
  if (toggles.forces)
    phase_forces();

  // Rule 5: Movement + collisions + annihilation
  if (toggles.movement)
    phase_movement();

  // Rule 5b: Absorbing-boundary sponge — disperse outgoing waves into the void
  // at the lattice faces. Runs AFTER gauss/forces/movement (the last flux
  // writers) so the damped edge shell is NOT refilled by the Gauss projection.
  // Gated; default off → golden-tick hash + conservation tests unchanged.
  if (toggles.absorbing_boundary)
    apply_absorbing_boundary(*this);


  // Self-field floor moved to Rule 3b (after Gauss, before forces) in Phase 3.
  // No second floor here — eliminates the double-injection energy leak.

  // Rule 6: Weak transmutation (polarity flip under field stress).
  // F5 (callstack audit 2026-04-17): extracted to weak_transmutation_cpu().
  if (toggles.weak_transmutation)
    weak_transmutation_cpu();

  // Rule 7: Triad binding detection (3 same-sign particles → locked).
  // F2 (callstack audit 2026-04-17): matching GPU path. No-op on CPU
  // until the triad-detection kernel is ported.
  if (toggles.triad_binding)
    triad_binding_cpu();

  // Rule 8: Proper time accumulation (gravity sector).
  // F5 (callstack audit 2026-04-17): extracted to accumulate_proper_time().
  if (toggles.latency_field)
    accumulate_proper_time();

  physical_time_ += dt_;
  ++tick_;

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

eft::DualCellContinuity RenderBridge::continuity_step() const {
  // ARCH-2-K (2026-04-25): const-method GPU branch routed through the
  // public gpu_engine_ptr() accessor (returns nullptr when no GPU).
#ifdef FTD_ENABLE_CUDA
  if (auto* gpu = const_cast<RenderBridge*>(this)->gpu_engine_ptr()) {
    return gpu->continuity_step();
  }
#endif
  return eft::DualCellContinuity(lattice_.size());
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
  // identical on CPU and GPU, tick by tick. The cost is one gpu_sync_to_host()
  // per tick (~3 MB at L=64; sub-ms on modern hardware). If a campaign needs
  // the old batched fast-path, call gpu_->run(N) directly via the public
  // engine accessor and skip the per-tick ledger.
  for (int i = 0; i < num_ticks; ++i) {
    tick();
  }
}

// ============================================================================
// Diagnostics / audits / EM decomposition — bodies in diagnostics_compute.cpp
// (R4, 2026-04-18). The methods below are thin wrappers.
// ============================================================================
Diagnostics RenderBridge::diagnostics() const { return ::ftd::compute_diagnostics(*this); }

EnergyAudit RenderBridge::energy_audit() const {
  EnergyAudit a = ::ftd::compute_energy_audit(*this);
  a.self_field_injection = self_field_injection_;  // private, stitched in here
  return a;
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
