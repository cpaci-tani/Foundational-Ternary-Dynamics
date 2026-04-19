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
 *   - No noetic/consciousness
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
#include "ftd/transmutation_phases.h"      // moved from mid-file to avoid nested-namespace include
#include "ftd/energy_ledger_compute.h"     // moved from mid-file to avoid nested-namespace include
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

RenderBridge::RenderBridge(int lattice_size)
    : lattice_(lattice_size), voxels_(lattice_.total_sites()),
      force_diag_(lattice_.total_sites()),
      delta_j_(lattice_.total_sites()),
      delta_j_L_(lattice_.total_sites()),
      delta_j_R_(lattice_.total_sites()),
      phi_(lattice_.total_sites(), 0.0),
      phi_coulomb_(lattice_.total_sites(), 0.0),
      phi_latency_(lattice_.total_sites(), 0.0),
      moved_(lattice_.total_sites(), 0),
      sor_source_(lattice_.total_sites(), 0.0)
{
    // PERF: pre-size per-tick scratch buffers so phase_write doesn't
    // construct ~5KB of mt19937 state per voxel. Under WASM (no OpenMP)
    // num_threads is always 1; native builds size to omp_get_max_threads().
    int num_threads = 1;
#ifdef _OPENMP
    num_threads = omp_get_max_threads();
#endif
    thread_seeds_.resize(num_threads, 0u);
    thread_rngs_.resize(num_threads);
    colored_sites_cache_.reserve(256);
#ifdef FTD_ENABLE_CUDA
    try {
        gpu_ = std::make_unique<gpu::GpuEngine>(lattice_size);
        use_gpu_ = true;
        std::cerr << "[RenderBridge] GPU backend active (CUDA, L=" << lattice_size << ")\n";
    } catch (const std::exception& e) {
        use_gpu_ = false;
        std::cerr << "[RenderBridge] GPU init failed: " << e.what() << " — using CPU\n";
    } catch (...) {
        use_gpu_ = false;
        std::cerr << "[RenderBridge] GPU init failed (unknown error) — using CPU\n";
    }
#endif
}

// Destructor must be in .cpp where GpuEngine is fully defined (unique_ptr needs it)
RenderBridge::~RenderBridge() = default;

#ifdef FTD_ENABLE_CUDA
void RenderBridge::gpu_sync_to_host() {
    if (use_gpu_ && gpu_dirty_) {
        gpu_->sync_to_host(voxels_);
        gpu_dirty_ = false;
    }
}

void RenderBridge::gpu_push_to_device() {
    if (use_gpu_) {
        gpu_->upload_from_host(voxels_);
        gpu_dirty_ = false;
    }
}

void RenderBridge::gpu_flush_host_mutations() {
    // Wave 5.2 (2026-04-14): tick() calls this at the start so that any
    // direct host writes done via voxels()[idx].field = ... since the
    // previous tick get pushed back to the GPU before physics runs.
    if (use_gpu_ && host_mutated_) {
        gpu_->upload_from_host(voxels_);
        gpu_dirty_ = false;
        host_mutated_ = false;
    }
}
#endif

// Wave 5 (2026-04-14): GPU-aware phi_latency accessor.
// When use_gpu_ is true, lazily fetches the latency Poisson potential
// from the GPU buffer (d_phi_latency). When use_gpu_ is false, returns
// the CPU SOR solver's cached vector directly.
const std::vector<double>& RenderBridge::phi_latency() const {
#ifdef FTD_ENABLE_CUDA
    if (use_gpu_ && gpu_) {
        // Mirror the GPU's phi_latency into our host vector so external
        // callers get a stable reference.
        const auto& gpu_phi = gpu_->phi_latency();
        auto& dst = const_cast<std::vector<double>&>(phi_latency_);
        dst = gpu_phi;
    }
#endif
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

void RenderBridge::phase_read() {
  const int N = static_cast<int>(lattice_.total_sites());
  const int L = lattice_.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  const bool do_wave = toggles.wave_propagation;
  const bool do_coupling = toggles.coupling;
  const bool dual = toggles.dual_substrate;
  const double cw2 = C_WAVE * C_WAVE;

  // Isotropic 18-point Laplacian weights.
  // For interior voxels we skip all modulo ops (same technique as sor_sweep_18pt):
  //   coord decomposition:  iz = i % L  (stride 1), iy = (i/L) % L (stride L), ix = i/LL (stride LL)
  //   interior iff all three coords ∈ [1, L-2].
  // For the ~(L-2)³/L³ ≈ 97.7% interior fraction (L=64), this eliminates every modulo.
  // Dual-substrate computes L and R Laplacians from the SAME 18 loaded neighbors — one
  // pass, no redundant neighbor lookups compared to calling laplacian_flux_L + laplacian_flux_R
  // separately.
  constexpr double INV3 = 1.0 / 3.0;
  constexpr double INV6 = 1.0 / 6.0;

  if (dual) {
    // Dual-substrate: compute delta for J_L and J_R in a single neighbor sweep
#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
      delta_j_L_[i] = {};
      delta_j_R_[i] = {};

      if (do_wave) {
        // Decompose flat index into lattice coordinates
        const int iz = i % L;
        const int iy = (i / L) % L;
        const int ix = i / LL;

        Vec3 lap_L, lap_R;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          // Interior fast path: precomputed offsets, zero modulo operations.
          // Neighbor offsets: ±1=±z, ±L=±y, ±LL=±x (matches lattice coord convention).
          const Vec3 fL = (voxels_[i+1].flux_L  + voxels_[i-1].flux_L
                         + voxels_[i+L].flux_L  + voxels_[i-L].flux_L
                         + voxels_[i+LL].flux_L + voxels_[i-LL].flux_L) * INV3;
          const Vec3 fR = (voxels_[i+1].flux_R  + voxels_[i-1].flux_R
                         + voxels_[i+L].flux_R  + voxels_[i-L].flux_R
                         + voxels_[i+LL].flux_R + voxels_[i-LL].flux_R) * INV3;
          const Vec3 eL = (voxels_[i+1+L].flux_L  + voxels_[i+1-L].flux_L
                         + voxels_[i-1+L].flux_L  + voxels_[i-1-L].flux_L
                         + voxels_[i+1+LL].flux_L + voxels_[i+1-LL].flux_L
                         + voxels_[i-1+LL].flux_L + voxels_[i-1-LL].flux_L
                         + voxels_[i+L+LL].flux_L + voxels_[i+L-LL].flux_L
                         + voxels_[i-L+LL].flux_L + voxels_[i-L-LL].flux_L) * INV6;
          const Vec3 eR = (voxels_[i+1+L].flux_R  + voxels_[i+1-L].flux_R
                         + voxels_[i-1+L].flux_R  + voxels_[i-1-L].flux_R
                         + voxels_[i+1+LL].flux_R + voxels_[i+1-LL].flux_R
                         + voxels_[i-1+LL].flux_R + voxels_[i-1-LL].flux_R
                         + voxels_[i+L+LL].flux_R + voxels_[i+L-LL].flux_R
                         + voxels_[i-L+LL].flux_R + voxels_[i-L-LL].flux_R) * INV6;
          lap_L = fL + eL - voxels_[i].flux_L * 4.0;
          lap_R = fR + eR - voxels_[i].flux_R * 4.0;
        } else {
          // Boundary slow path: modular wrapping via lattice neighbor tables.
          lap_L = ::ftd::laplacian_field<&Voxel::flux_L>(voxels_, lattice_, i);
          lap_R = ::ftd::laplacian_field<&Voxel::flux_R>(voxels_, lattice_, i);
        }
        delta_j_L_[i] = lap_L * cw2;
        delta_j_R_[i] = lap_R * cw2;
      }

      // Coupling source: split equally between L and R substrates
      if (do_coupling) {
        Vec3 grad_s = gradient_state(i) * (G_C * 0.5);
        Vec3 curl_sv = curl_state_velocity(i) * (G_C * 0.5);
        delta_j_L_[i] += grad_s + curl_sv;
        delta_j_R_[i] += grad_s + curl_sv;
      }
    }
  } else {
    // Single-substrate: inline Laplacian with the same interior/boundary split
#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
      delta_j_[i] = {};

      if (do_wave) {
        const int iz = i % L;
        const int iy = (i / L) % L;
        const int ix = i / LL;

        Vec3 lap;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          // Interior fast path
          const Vec3 f = (voxels_[i+1].flux  + voxels_[i-1].flux
                        + voxels_[i+L].flux  + voxels_[i-L].flux
                        + voxels_[i+LL].flux + voxels_[i-LL].flux) * INV3;
          const Vec3 e = (voxels_[i+1+L].flux  + voxels_[i+1-L].flux
                        + voxels_[i-1+L].flux  + voxels_[i-1-L].flux
                        + voxels_[i+1+LL].flux + voxels_[i+1-LL].flux
                        + voxels_[i-1+LL].flux + voxels_[i-1-LL].flux
                        + voxels_[i+L+LL].flux + voxels_[i+L-LL].flux
                        + voxels_[i-L+LL].flux + voxels_[i-L-LL].flux) * INV6;
          lap = f + e - voxels_[i].flux * 4.0;
        } else {
          // Boundary slow path
          lap = laplacian_flux(i);
        }
        delta_j_[i] = lap * cw2;
      }

      if (do_coupling) {
        delta_j_[i] += gradient_state(i) * G_C;
        delta_j_[i] += curl_state_velocity(i) * G_C;
      }
    }
  }
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

void RenderBridge::phase_write() {
  const int N = static_cast<int>(lattice_.total_sites());
  const double damping_factor = (dt_ > 1.0001)
      ? std::pow(1.0 - DAMPING, dt_)
      : 1.0 - DAMPING;
  const bool do_damping = toggles.damping;
  const bool do_genesis = toggles.genesis;
  const bool selective = toggles.selective_damping;
  const bool do_larmor = toggles.larmor_radiation;
  const bool dual = toggles.dual_substrate;

  // Phase D: Precompute near-particle mask (O(N), race-free)
  if (selective) {
    near_particle_.resize(N, 0);
    std::fill(near_particle_.begin(), near_particle_.end(), 0);
    if (do_larmor) {
      near_accel_.resize(N, 0.0);
      std::fill(near_accel_.begin(), near_accel_.end(), 0.0);
    }
    for (int i = 0; i < N; ++i) {
      if (voxels_[i].state != 0) {
        near_particle_[i] = 1;
        if (do_larmor) {
          double a = voxels_[i].accel_mag;
          near_accel_[i] = std::max(near_accel_[i], a);
        }
        for (int n : lattice_.neighbors_6(i)) {
          near_particle_[n] = 1;
          if (do_larmor) {
            double a = voxels_[i].accel_mag;  // particle's acceleration
            near_accel_[n] = std::max(near_accel_[n], a);
          }
        }
      }
    }
  }

  // PERF: re-seed the per-thread RNG pool ONCE per phase_write (was
  // constructing a fresh ~5KB mt19937 per voxel inside the parallel-for,
  // costing ~1.3 GB/tick of stack churn at L=64). Buffers are bridge
  // members presized in the ctor — no allocation here.
  int num_threads = 1;
#ifdef _OPENMP
  num_threads = omp_get_max_threads();
#endif
  for (int t = 0; t < num_threads; ++t) {
    thread_seeds_[t] = rng_();
    thread_rngs_[t].seed(thread_seeds_[t]);
  }

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];

    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    auto& local_rng = thread_rngs_[tid];
    std::uniform_real_distribution<double> local_uniform(0.0, 1.0);

    const bool should_damp = !selective || near_particle_[i];

    if (dual) {
      // ---- Dual-substrate leapfrog integration ----
      v.wave_vel_L += delta_j_L_[i];
      v.wave_vel_R += delta_j_R_[i];
      v.flux_L += v.wave_vel_L;
      v.flux_R += v.wave_vel_R;

      // Damping on both substrates independently
      if (do_damping && should_damp) {
        double eff_damping = damping_factor;
        // Larmor radiation: modulate damping at ALL near-particle sites
        // Uses max accel_mag of nearby particles (propagated via near_accel_)
        // Static charges (a=0) → LARMOR_FLOOR ≈ 1% damping
        // Accelerating charges → enhanced damping ∝ a²
        if (do_larmor && selective && near_particle_[i]) {
          double a2 = near_accel_[i] * near_accel_[i];
          double larmor_mod = std::min(1.0, LARMOR_FLOOR + K_LARMOR * a2);
          eff_damping = 1.0 - DAMPING * larmor_mod;
        }
        v.flux_L *= eff_damping;
        v.flux_R *= eff_damping;
        v.wave_vel_L *= eff_damping;
        v.wave_vel_R *= eff_damping;
      }

      // Update observable field: flux = J_L + J_R
      v.flux = v.flux_L + v.flux_R;
      v.wave_vel = v.wave_vel_L + v.wave_vel_R;

      // Genesis: use chirality density for polarity (dual-substrate rule)
      // chi = |psi_L|^2 - |psi_R|^2 where psi_X = J_Xx + i*J_Xy
      // s = sgn(chi) * Theta(|chi| - K_B^2)
      if (do_genesis && v.state == 0 && v.density() > K_GENESIS) {
        double excess = v.density() - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_B);
        if (local_uniform(local_rng) < p) {
          double chi = v.chirality_density();
          v.state = (chi >= 0) ? 1 : -1;
          int pid;
#pragma omp critical(genesis_id)
          { pid = next_particle_id_++; }
          v.particle_id = pid;

          // Spin from curl of observable J
          Vec3 curl = curl_flux(i);
          double ax = std::abs(curl.x), ay = std::abs(curl.y), az = std::abs(curl.z);
          double mx = std::max({ax, ay, az});
          if (mx > EPSILON_MAG) {
            if (az >= ax && az >= ay) v.spin = (curl.z > 0) ? 1 : -1;
            else if (ay >= ax) v.spin = (curl.y > 0) ? 1 : -1;
            else v.spin = (curl.x > 0) ? 1 : -1;
          } else {
            v.spin = (local_uniform(local_rng) < 0.5) ? 1 : -1;
          }

          // Color from dominant flux axis
          double fx = std::abs(v.flux.x), fy = std::abs(v.flux.y), fz = std::abs(v.flux.z);
          if (fx >= fy && fx >= fz) v.color = 1;
          else if (fy >= fx && fy >= fz) v.color = 2;
          else v.color = 3;
        }
      }
    } else {
      // ---- Single-substrate leapfrog integration (non-dual path) ----
      v.wave_vel += delta_j_[i];
      v.flux += v.wave_vel;

      if (do_damping && should_damp) {
        double eff_damping = damping_factor;
        // Larmor radiation: modulate damping at ALL near-particle sites
        if (do_larmor && selective && near_particle_[i]) {
          double a2 = near_accel_[i] * near_accel_[i];
          double larmor_mod = std::min(1.0, LARMOR_FLOOR + K_LARMOR * a2);
          eff_damping = 1.0 - DAMPING * larmor_mod;
        }
        v.flux *= eff_damping;
        v.wave_vel *= eff_damping;
      }

      // Genesis: void + high flux → manifest (div(J) for polarity)
      if (do_genesis && v.state == 0 && v.density() > K_GENESIS) {
        double excess = v.density() - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_B);
        if (local_uniform(local_rng) < p) {
          double div = divergence_flux(i);
          v.state = (div > 0) ? 1 : -1;
          int pid;
#pragma omp critical(genesis_id)
          { pid = next_particle_id_++; }
          v.particle_id = pid;

          Vec3 curl = curl_flux(i);
          double ax = std::abs(curl.x), ay = std::abs(curl.y), az = std::abs(curl.z);
          double mx = std::max({ax, ay, az});
          if (mx > EPSILON_MAG) {
            if (az >= ax && az >= ay) v.spin = (curl.z > 0) ? 1 : -1;
            else if (ay >= ax) v.spin = (curl.y > 0) ? 1 : -1;
            else v.spin = (curl.x > 0) ? 1 : -1;
          } else {
            v.spin = (local_uniform(local_rng) < 0.5) ? 1 : -1;
          }

          double fx = std::abs(v.flux.x), fy = std::abs(v.flux.y), fz = std::abs(v.flux.z);
          if (fx >= fy && fx >= fz) v.color = 1;
          else if (fy >= fx && fy >= fz) v.color = 2;
          else v.color = 3;
        }
      }
    }

    // Evaporation: low TOTAL wave energy → return to void
    // (same for both single and dual substrate — uses observable flux)
    constexpr double EVAP_ENERGY = K_B * K_B * EVAP_THRESHOLD;
    double local_energy = v.flux.mag2() + v.wave_vel.mag2();
    {
      const auto& nbrs = lattice_.neighbors_6(i);
      for (int n : nbrs)
        local_energy += voxels_[n].flux.mag2() + voxels_[n].wave_vel.mag2();
    }
    if (do_genesis && v.state != 0
        && local_energy < EVAP_ENERGY
        && !v.locked) {
      v.state = 0;
      v.particle_id = -1;
      v.spin = 0;
      v.color = 0;
    }
  }
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
  gauss_project_cpu(voxels_, phi_, sor_source_, lattice_, toggles.dual_substrate);
}

void RenderBridge::solve_coulomb_poisson() {
  solve_coulomb_poisson_cpu(voxels_, phi_coulomb_, sor_source_, lattice_);
}

void RenderBridge::solve_latency_poisson() {
  solve_latency_poisson_cpu(voxels_, phi_latency_, sor_source_, lattice_);
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
  const int N = static_cast<int>(lattice_.total_sites());
  const int L = lattice_.size();

  // Solve Coulomb potential (warm-started SOR)
  // Skip when emergent_forces is ON — force comes from flux field directly
  if (toggles.poisson_coulomb && !toggles.emergent_forces)
    solve_coulomb_poisson();

  // ── Color force ────────────────────────────────────────────────────
  //
  // EPISTEMIC STATUS: [PHENOMENOLOGICAL FIT], not [EMERGENT].
  //
  // What IS emergent from the FTD dynamics:
  //   - Z₃ colour LABELLING — assigned by dominant flux axis per voxel.
  //   - Confinement of like-colour configurations at distance x₋ ≈ 3.024.
  //
  // What is NOT emergent — these pieces are hand-fit to reproduce QCD:
  //   - Force coefficient cf = {+0.5 same-colour, −1 different} — SU(3)
  //     Casimir-motivated, inserted numerically.
  //   - Three-regime piecewise profile (Coulomb / flux-tube / linear) —
  //     hand-spliced at COLOR_COULOMB_RADIUS / COLOR_TRANSITION_RADIUS.
  //   - Running coupling α_s(r) — standard one-loop QCD form.
  //
  // Reading this code as "FTD derives QCD confinement" is wrong. What's
  // actually happening is: FTD supplies a 3-colour labelling; a QCD-shaped
  // potential is then built by hand on top. The shape is imposed.
  //
  // To upgrade this to a genuine derivation, replace the piecewise force
  // law with a dynamical SU(3) gauge field whose Wilson-loop expectation
  // produces linear confinement at large r without a hand-inserted regime
  // switch. That work is [OPEN].
  // PERF: colored_sites_cache_ is a bridge member — clear+push reuses capacity,
  // no per-tick malloc.
  colored_sites_cache_.clear();
  if (toggles.color_forces) {
    for (int ii = 0; ii < N; ++ii) {
      if (voxels_[ii].state != 0 && voxels_[ii].color != 0) {
        auto cc = lattice_.coord(ii);
        colored_sites_cache_.push_back({cc.x, cc.y, cc.z,
                                        voxels_[ii].state, voxels_[ii].color});
      }
    }
  }

  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];
    if (v.state == 0) continue;

    // EM force: three modes
    //   1. Poisson-based:  F = -alpha·s·∇φ_C        (standard, most accurate)
    //   2. Legacy gradient: F = -alpha·s·∇(∇·J)      (direct, short-range)
    //   3. Emergent (EFT):  F = G_C·s·∇|J|_{tier2}   (force from flux field)
    //      In mode 3, alpha = G_C² emerges: one G_C from this probe coupling,
    //      one G_C already embedded in the flux amplitude from the wave equation.
    Vec3 f_em;
    if (toggles.emergent_forces) {
      // EFT emergent force: read force FROM the flux field established by
      // wave equation + Gauss constraint. No Poisson solver needed.
      // Use tier-2 stencil (r=2 neighbors) to avoid self-field contamination.
      auto ci = lattice_.coord(i);
      int L = lattice_.size();
      double grad_x = 0, grad_y = 0, grad_z = 0;
      // Tier-2 finite differences (skip r=1 to avoid self-field wake)
      auto safe = [&](int x, int y, int z) -> double {
        int wx = ((x % L) + L) % L;
        int wy = ((y % L) + L) % L;
        int wz = ((z % L) + L) % L;
        return voxels_[lattice_.index(wx, wy, wz)].density();
      };
      grad_x = (safe(ci.x+2, ci.y, ci.z) - safe(ci.x-2, ci.y, ci.z)) * 0.25;
      grad_y = (safe(ci.x, ci.y+2, ci.z) - safe(ci.x, ci.y-2, ci.z)) * 0.25;
      grad_z = (safe(ci.x, ci.y, ci.z+2) - safe(ci.x, ci.y, ci.z-2)) * 0.25;
      Vec3 grad_rho_t2 = {grad_x, grad_y, grad_z};
      // Force = G_C · state · ∇|J| (one vertex coupling; other G_C in flux)
      f_em = grad_rho_t2 * (G_C * v.state);
    } else if (toggles.poisson_coulomb) {
      Vec3 grad_phi = gradient_scalar(i, phi_coulomb_);
      f_em = grad_phi * (-ALPHA * v.state);       // ALPHA == ALPHA_EFT (G_C² identity)
    } else {
      Vec3 grad_divJ = gradient_divergence(i);
      f_em = grad_divJ * (-ALPHA * v.state);       // ALPHA == ALPHA_EFT (G_C² identity)
    }

    // Gravitational force from density gradient
    // Use tier-2 (r=2) stencil for manifested particles to avoid
    // self-field contamination at tier-1 (r=1) face-neighbors.
    // The self-field wake at r=1 creates an asymmetric density gradient
    // that causes spurious self-acceleration. At r=2 the self-field
    // influence is negligible and only external gradients contribute.
    Vec3 f_grav;
    if (toggles.gravity) {
      auto c = lattice_.coord(i);
      double dx = voxels_[lattice_.index(c.x+2, c.y, c.z)].density()
                - voxels_[lattice_.index(c.x-2, c.y, c.z)].density();
      double dy = voxels_[lattice_.index(c.x, c.y+2, c.z)].density()
                - voxels_[lattice_.index(c.x, c.y-2, c.z)].density();
      double dz = voxels_[lattice_.index(c.x, c.y, c.z+2)].density()
                - voxels_[lattice_.index(c.x, c.y, c.z-2)].density();
      Vec3 grad_rho = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
      f_grav = grad_rho * G_N;
    }

    // Lorentz (magnetic) force: F = α·s·(v × B) where B = curl(J)
    // From the Lagrangian velocity-coupling term L_vc = -g_c·s·(v·J),
    // the E-L equation yields F = g_c·q·(v × curl(J)).
    // With coupling g_c² = α, this gives F = α·s·(v × B).
    Vec3 f_lorentz;
    if (toggles.lorentz_force && v.speed() > EPSILON_MAG) {
      Vec3 B = curl_flux(i);
      f_lorentz = Vec3::cross(v.velocity, B) * (ALPHA * v.state);       // ALPHA == ALPHA_EFT
    }

    // ── Color force: pairwise SU(3)-inspired interaction ─────────────
    // F_color(i←j) = α_s(r) · color_factor(c_i, c_j) · r̂ / r²
    // Running coupling α_s(r) implements asymptotic freedom at short r
    // and confinement saturation at large r.
    Vec3 f_color;
    if (toggles.color_forces && v.color != 0) {
      auto ci = lattice_.coord(i);
      for (auto& cs : colored_sites_cache_) {
        // Skip self via coord equality (cheaper than carrying idx)
        if (cs.cx == ci.x && cs.cy == ci.y && cs.cz == ci.z) continue;
        double ddx = cs.cx - ci.x;
        double ddy = cs.cy - ci.y;
        double ddz = cs.cz - ci.z;
        if (ddx >  L/2) ddx -= L;
        if (ddx < -L/2) ddx += L;
        if (ddy >  L/2) ddy -= L;
        if (ddy < -L/2) ddy += L;
        if (ddz >  L/2) ddz -= L;
        if (ddz < -L/2) ddz += L;
        double r2 = ddx*ddx + ddy*ddy + ddz*ddz;
        double r = std::sqrt(r2);
        if (r < 1.0) r = 1.0;  // Clamp to lattice spacing (matches GPU)
        r2 = r * r;
        double cf = (v.color == cs.color) ? 0.5 : -1.0;
        double as = alpha_s_lattice(r);

        // Three-regime force profile (matches GPU kernels_forces.cu):
        //   r < COLOR_COULOMB_RADIUS:    Coulomb (asymptotic freedom)
        //   transition:                  Flux tube stretching
        //   r >= COLOR_TRANSITION_RADIUS: Linear confinement (constant string tension)
        double F_mag;
        if (r < COLOR_COULOMB_RADIUS) {
          F_mag = as * cf / r2;
        } else if (r < COLOR_TRANSITION_RADIUS) {
          F_mag = as * cf / (COLOR_TRANSITION_DENOM * r);
        } else {
          F_mag = as * cf * r / COLOR_LINEAR_DENOM;
        }

        // ddx points from probe to source; negate for repulsive force direction
        // Same color (cf>0): force pushes AWAY from source (repulsive)
        // Diff color (cf<0): force pulls TOWARD source (attractive)
        f_color.x -= F_mag * ddx / r;
        f_color.y -= F_mag * ddy / r;
        f_color.z -= F_mag * ddz / r;
      }
    }

    Vec3 f_total = f_em + f_grav + f_lorentz + f_color;

    // Store for diagnostics
    force_diag_[i].f_coulomb = f_em;
    force_diag_[i].f_gravity = f_grav;
    force_diag_[i].f_strong = f_color;
    force_diag_[i].f_magnetic = f_lorentz;
    force_diag_[i].f_exchange = {};

    // Record acceleration magnitude
    v.accel_mag = f_total.mag();

    // Apply force (skip locked particles)
    if (!v.locked) {
      // ── γ_FTD MOMENTUM INTEGRATION (2026-04-17, TRACKER §1.2) ──────
      //
      // FTD bandwidth postulate: v²/C² + L² < 1, where C = C_SPEED and
      // L is local topological latency (gravity).  The corresponding
      // Lorentz factor is γ_FTD = 1/√(1 − v²/C² − L²).
      //
      // To respect this constraint exactly, we integrate MOMENTUM, not
      // velocity: p = γ_FTD · v.  Newton's law becomes dp/dt = F, and
      // v is extracted from p at the end of the step.  This guarantees
      // |v| → C·√(1 − L²) asymptotically as force → ∞; no clamp, no
      // energy discard, Lorentz-invariant by construction.
      //
      // Algebra (derivation in TRACKER §1.2):
      //   γ²|v|² = |p|²
      //   γ² = 1/(1 − |v|²/C² − L²)
      //   ⇒  |v|² = C²(1 − L²) · |p|² / (C² + |p|²)
      //   ⇒  v⃗   = p⃗ · C · √((1 − L²) / (C² + |p|²))
      //
      // Newtonian limit (|v| << C, L = 0): γ → 1, p ≈ v, v_new ≈ v + F·dt. ✓
      // Ultra-relativistic (|p| → ∞):       |v| → C·√(1 − L²).          ✓
      // Horizon (L → 1):                    |v| → 0.                     ✓
      //
      // Superseded the previous non-relativistic clamp
      // `if (|v| > C) v *= C/|v|;` which discarded energy and was also
      // STRICTER than the true bandwidth (clamp allowed |v| ≤ C(1−L²);
      // FTD bandwidth allows |v| ≤ C·√(1−L²)).
      const double C      = C_SPEED;
      const double C2     = C * C;
      const double L      = v.latency;                  // 0 if latency_field off
      const double L2     = L * L;
      // Budget-safe: clamp 1−L² strictly positive so sqrt() never
      // underflows at or near the horizon.
      const double one_L2 = std::max(1.0 - L2, 1e-6);

      // Current γ (with a budget floor of 1e-6 to keep γ finite if the
      // previous tick left v at the bandwidth edge).
      const double v2 = v.velocity.mag2();
      double budget  = v2 / C2 + L2;
      if (budget > 1.0 - 1e-6) budget = 1.0 - 1e-6;
      const double gamma_in = 1.0 / std::sqrt(1.0 - budget);

      // Reconstruct momentum, apply force, extract new velocity.
      Vec3 p = v.velocity * gamma_in;
      p = p + f_total * dt_;
      const double p2 = p.mag2();
      const double scale = C * std::sqrt(one_L2 / (C2 + p2));
      v.velocity = p * scale;
    }
  }
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

void RenderBridge::phase_movement() {
  const int N = static_cast<int>(lattice_.total_sites());
  std::fill(moved_.begin(), moved_.end(), 0);

  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];
    if (v.state == 0 || v.locked || moved_[i]) continue;

    v.remainder += v.velocity * dt_;

    auto c = lattice_.coord(i);
    int dx = 0, dy = 0, dz = 0;

    if (v.remainder.x >= 1.0) { dx = 1; v.remainder.x -= 1.0; }
    else if (v.remainder.x <= -1.0) { dx = -1; v.remainder.x += 1.0; }
    if (v.remainder.y >= 1.0) { dy = 1; v.remainder.y -= 1.0; }
    else if (v.remainder.y <= -1.0) { dy = -1; v.remainder.y += 1.0; }
    if (v.remainder.z >= 1.0) { dz = 1; v.remainder.z -= 1.0; }
    else if (v.remainder.z <= -1.0) { dz = -1; v.remainder.z += 1.0; }

    if (dx == 0 && dy == 0 && dz == 0) continue;

    int target = lattice_.index(c.x + dx, c.y + dy, c.z + dz);
    auto &t = voxels_[target];

    if (t.state == 0) {
      // Move: transfer particle to target
      t.state = v.state;
      t.velocity = v.velocity;
      t.remainder = v.remainder;
      t.pair_id = v.pair_id;
      t.accel_mag = v.accel_mag;
      t.spin = v.spin;
      t.color = v.color;
      t.particle_id = v.particle_id;

      // Portable self-field: particle carries flux with it (up to K_B)
      double old_rho = v.density();
      if (old_rho > EPSILON_MAG) {
        double transfer = std::min(old_rho, K_B);
        double frac = transfer / old_rho;
        Vec3 self_field = v.flux * frac;
        v.flux = v.flux - self_field;
        t.flux = t.flux + self_field;

        // Dual-substrate: carry proportional L/R flux too
        if (toggles.dual_substrate) {
          Vec3 sf_L = v.flux_L * frac;
          Vec3 sf_R = v.flux_R * frac;
          v.flux_L = v.flux_L - sf_L;
          v.flux_R = v.flux_R - sf_R;
          t.flux_L = t.flux_L + sf_L;
          t.flux_R = t.flux_R + sf_R;
        }
      }

      v.state = 0;
      v.velocity = {};
      v.remainder = {};
      v.pair_id = -1;
      v.particle_id = -1;
      v.spin = 0;
      v.color = 0;
      moved_[target] = 1;  // Prevent re-processing this tick
    } else if (t.state == v.state) {
      // Same sign: elastic bounce
      if (dx != 0) v.velocity.x *= -1.0;
      if (dy != 0) v.velocity.y *= -1.0;
      if (dz != 0) v.velocity.z *= -1.0;
      v.remainder = {};
    } else {
      // Opposite sign: annihilation — both particles return to void.
      Vec3 flux_v = v.flux;
      Vec3 flux_t = t.flux;
      Vec3 flux_v_L, flux_v_R, flux_t_L, flux_t_R;
      if (toggles.dual_substrate) {
        flux_v_L = v.flux_L; flux_v_R = v.flux_R;
        flux_t_L = t.flux_L; flux_t_R = t.flux_R;
      }
      v.state = 0; t.state = 0;
      v.velocity = {}; t.velocity = {};
      v.remainder = {}; t.remainder = {};
      v.pair_id = -1; t.pair_id = -1;
      v.particle_id = -1; t.particle_id = -1;
      v.accel_mag = 0.0; t.accel_mag = 0.0;
      v.spin = 0; v.color = 0;
      t.spin = 0; t.color = 0;
      v.flux = {}; t.flux = {};
      if (toggles.dual_substrate) {
        v.flux_L = {}; v.flux_R = {};
        t.flux_L = {}; t.flux_R = {};
      }
      // Distribute each particle's flux to its own neighbors
      auto nbrs_v = lattice_.neighbors_6(i);
      auto nbrs_t = lattice_.neighbors_6(target);
      for (int n : nbrs_v) voxels_[n].flux += flux_v * (1.0 / 6.0);
      for (int n : nbrs_t) voxels_[n].flux += flux_t * (1.0 / 6.0);
      if (toggles.dual_substrate) {
        for (int n : nbrs_v) {
          voxels_[n].flux_L += flux_v_L * (1.0 / 6.0);
          voxels_[n].flux_R += flux_v_R * (1.0 / 6.0);
        }
        for (int n : nbrs_t) {
          voxels_[n].flux_L += flux_t_L * (1.0 / 6.0);
          voxels_[n].flux_R += flux_t_R * (1.0 / 6.0);
        }
      }
    }
  }
}

// ============================================================================
// The Tick: Six rules, executed in order
// ============================================================================

void RenderBridge::tick() {
  // F3 (callstack audit 2026-04-17): validate runs on BOTH paths now
  // so toggle-combination warnings surface regardless of CPU/GPU build.
  {
      std::string validErr;
      if (!toggles.validate(&validErr))
          std::cerr << "[TermToggles] Invalid combination: " << validErr;
  }

#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    // Wave 5.2: flush any host-side mutations (e.g. test doing
    // voxels()[idx].locked = true) back to the GPU before physics runs.
    gpu_flush_host_mutations();
    // Sync toggles to GPU engine
    gpu_->toggles = toggles;
    gpu_->tick();
    gpu_dirty_ = true;
    physical_time_ += dt_;
    ++tick_;

    // ── GPU EnergyLedger + proper-time (TRACKER §1.7 + audit F4) ──
    // Sync voxels back to host, then populate both the ledger and
    // (if latency_field is on) the per-particle proper-time tau.
    //
    // Cost: one PCIe download per tick (≈ 3 MB at L=64, sub-ms on
    // modern hardware; far below a CUDA tick's physics cost). If this
    // becomes a bottleneck for long CUDA benchmarks, replace with a
    // device-side reduction kernel that returns just the three scalar
    // sums (E_field, E_wave, E_kin) — see comment in gpu_engine.cu.
    gpu_sync_to_host();
    if (toggles.latency_field)
      accumulate_proper_time();
    update_energy_ledger();
    return;
  }
#endif

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

void RenderBridge::run(int num_ticks) {
#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    gpu_->toggles = toggles;
    gpu_->run(num_ticks);
    gpu_dirty_ = true;
    physical_time_ += dt_ * num_ticks;
    tick_ += num_ticks;
    return;
  }
#endif
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
