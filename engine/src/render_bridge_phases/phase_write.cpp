/**
 * @file engine/src/render_bridge_phases/phase_write.cpp
 * @purpose Implementation of phase_write decomposition (Phase 4a, 2026-04-27).
 *
 * Extracted from render_bridge.cpp following the R1-R5 precedent
 * (poisson_solvers.cpp, transmutation_phases.cpp, energy_ledger_compute.cpp,
 * diagnostics_compute.cpp, injection.cpp). See ADR-0008.
 *
 * The original phase_write() was ~265 LOC mixing:
 *   - prologue: damping factor + selective-damping mask + larmor near-accel
 *   - flux pre-write snapshot for race-free genesis curl reads
 *   - per-thread RNG (Langevin) seeding
 *   - main parallel-for: leapfrog (dual or single) + damping/Langevin + genesis + evaporation
 *   - sequential post-pass: pending-particle-id assignment
 *
 * The extraction preserves the parallel-for body BYTE-IDENTICAL. The golden
 * tick test (test_render_bridge_golden) hashes 100 ticks to
 * 0xcd957b601d47868a and is the strict gate on this refactor: any drift
 * here is a physics bug.
 *
 * RF-4 deduplication: the manifest-at body (state, particle_id sentinel,
 * spin from curl, color from dominant flux axis) was byte-identical between
 * the dual-substrate and single-substrate genesis paths. It is now a single
 * static helper `manifest_at()` called from both branches.
 */

#include "ftd/render_bridge_phases.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/sublattice.h"
#include "ftd/field_operators.h"
#include "ftd/bridge_rng.h"
#include "ftd/voxel_rng.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace ftd {

// VoxelRng enum + voxel_uniform now live in `engine/include/ftd/voxel_rng.h`
// (BH-F5/F8/F9 closure 2026-05-05). The shared header provides byte-equivalent
// SplitMix64 streams to both CPU and GPU paths so per-voxel CPU↔GPU parity
// holds bit-exactly under stochastic toggles. No behavior change in this
// commit relative to the prior anonymous-namespace local definitions; the
// arithmetic is identical (verified by golden hash bit-exactness).

namespace {

// RF-4 dedup: shared manifest body. Caller has already determined polarity
// via either chirality density (dual) or flux divergence (single) and
// passed it in as `polarity_signal`. This helper assigns state, marks the
// particle_id sentinel, derives spin from the pre-write flux curl, and
// derives color from the dominant flux axis — byte-identical with the
// original phase_write() inline blocks.
inline void manifest_at(RenderBridge& rb,
                        Voxel& v,
                        double polarity_signal,
                        const std::vector<Vec3>& flux_pre,
                        const Lattice& lattice,
                        int i,
                        std::uint64_t gseed,
                        int tick,
                        bool dual) {
  // Dual path uses chirality sign convention (>= 0 → +1); single path
  // uses divergence sign convention (> 0 → +1). The two conventions are
  // byte-equivalent for non-zero polarity signals; the dual path's `>= 0`
  // assigns +1 at exactly 0 while the single path's `> 0` assigns -1 at
  // exactly 0. Preserve that by branching on `dual`.
  if (dual) {
    rb.set_state(i, static_cast<int8_t>((polarity_signal >= 0) ? 1 : -1));
  } else {
    rb.set_state(i, static_cast<int8_t>((polarity_signal > 0) ? 1 : -1));
  }
  // ARCH-7 (2026-04-25): defer particle_id assignment until the sequential
  // post-pass so IDs match voxel-index order regardless of OMP scheduling.
  v.particle_id = -2;

  // ARCH-7b: spin from curl of the pre-write flux snapshot — sibling
  // thread writes don't race the curl read.
  Vec3 curl = ::ftd::curl_from_flux_array(flux_pre, lattice, i);
  double ax = std::abs(curl.x), ay = std::abs(curl.y), az = std::abs(curl.z);
  double mx = std::max({ax, ay, az});
  if (mx > EPSILON_MAG) {
    if (az >= ax && az >= ay) v.spin = (curl.z > 0) ? 1 : -1;
    else if (ay >= ax) v.spin = (curl.y > 0) ? 1 : -1;
    else v.spin = (curl.x > 0) ? 1 : -1;
  } else {
    v.spin = (voxel_uniform(gseed, i, tick,
                            static_cast<std::uint64_t>(VoxelRng::GenesisSpin)) < 0.5) ? 1 : -1;
  }

  // Color from dominant flux axis (uses live flux, not pre-write snapshot).
  double fx = std::abs(v.flux.x), fy = std::abs(v.flux.y), fz = std::abs(v.flux.z);
  if (fx >= fy && fx >= fz) v.color = 1;
  else if (fy >= fx && fy >= fz) v.color = 2;
  else v.color = 3;
}
}  // namespace

// =============================================================================
// Public free-function entry points
// =============================================================================

void snapshot_flux_pre_write(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  rb.flux_pre_write_.resize(N);
  for (int i = 0; i < N; ++i) rb.flux_pre_write_[i] = rb.voxels_[i].flux;
}

void compute_near_particle_mask(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  const bool do_larmor = rb.toggles.larmor_radiation;

  rb.near_particle_.resize(N, 0);
  std::fill(rb.near_particle_.begin(), rb.near_particle_.end(), 0);
  if (do_larmor) {
    rb.near_accel_.resize(N, 0.0);
    std::fill(rb.near_accel_.begin(), rb.near_accel_.end(), 0.0);
  }
  for (int i : rb.ordered_active_indices()) {
    if (rb.voxels_[i].state != 0) {
      rb.near_particle_[i] = 1;
      if (do_larmor) {
        double a = rb.voxels_[i].accel_mag;
        rb.near_accel_[i] = std::max(rb.near_accel_[i], a);
      }
      for (int n : rb.lattice_.neighbors_6(i)) {
        rb.near_particle_[n] = 1;
        if (do_larmor) {
          double a = rb.voxels_[i].accel_mag;  // particle's acceleration
          rb.near_accel_[n] = std::max(rb.near_accel_[n], a);
        }
      }
    }
  }
}

void phase_write_main_loop(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  const double damping_factor = (rb.dt_ > 1.0001)
      ? std::pow(1.0 - DAMPING, rb.dt_)
      : 1.0 - DAMPING;
  const bool do_damping = rb.toggles.damping;
  const bool do_genesis = rb.toggles.genesis;
  const bool do_evaporation = rb.toggles.evaporation;
  const bool selective = rb.toggles.selective_damping;
  const bool do_larmor = rb.toggles.larmor_radiation;
  const bool dual = rb.toggles.dual_substrate;

  // PERF: re-seed the per-thread RNG pool ONCE per phase_write (was
  // constructing a fresh ~5KB mt19937 per voxel inside the parallel-for).
  int num_threads = 1;
#ifdef _OPENMP
  num_threads = omp_get_max_threads();
#endif
  if (rb.toggles.langevin
      && (!rb.langevin_seed_initialized_
          || rb.active_langevin_seed_ != rb.toggles.langevin_seed)) {
    rb.rng_state_->seed(rb.toggles.langevin_seed);
    rb.active_langevin_seed_ = rb.toggles.langevin_seed;
    rb.langevin_seed_initialized_ = true;
  }
  if (static_cast<int>(rb.thread_seeds_.size()) < num_threads)
    rb.thread_seeds_.resize(num_threads, 0u);
  rb.rng_state_->reseed_thread_pool(rb.thread_seeds_.data(),
                                    static_cast<std::size_t>(num_threads));

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    auto &v = rb.voxels_[i];

    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    BridgeRng& rng = *rb.rng_state_;
    const std::size_t tids = static_cast<std::size_t>(tid);

    const bool should_damp = !selective || rb.near_particle_[i];

    if (dual) {
      // ---- Dual-substrate leapfrog integration ----
      if (rb.toggles.symplectic_leapfrog) {
        v.wave_vel_L += rb.delta_j_L_[i] * rb.dt_;
        v.wave_vel_R += rb.delta_j_R_[i] * rb.dt_;
        v.flux_L += v.wave_vel_L * rb.dt_;
        v.flux_R += v.wave_vel_R * rb.dt_;
      } else {
        v.wave_vel_L += rb.delta_j_L_[i];
        v.wave_vel_R += rb.delta_j_R_[i];
        v.flux_L += v.wave_vel_L;
        v.flux_R += v.wave_vel_R;
      }

      // Damping on both substrates independently
      if (do_damping && should_damp) {
        double eff_damping = damping_factor;
        // Larmor radiation: modulate damping at ALL near-particle sites
        if (do_larmor && selective && rb.near_particle_[i]) {
          double a2 = rb.near_accel_[i] * rb.near_accel_[i];
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

      // Genesis (dual): chirality density for polarity.
      if (do_genesis && v.state == 0 && v.density() > K_GENESIS) {
        double excess = v.density() - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_B);
        const std::uint64_t gseed =
            static_cast<std::uint64_t>(rb.toggles.langevin_seed);
        if (voxel_uniform(gseed, i, rb.tick_,
                          static_cast<std::uint64_t>(VoxelRng::GenesisManifest)) < p) {
          double chi = v.chirality_density();
          manifest_at(rb, v, chi, rb.flux_pre_write_, rb.lattice_, i, gseed, rb.tick_, /*dual=*/true);
        }
      }
    } else {
      // ---- Single-substrate leapfrog integration (non-dual path) ----
      if (rb.toggles.symplectic_leapfrog) {
        v.wave_vel += rb.delta_j_[i] * rb.dt_;
        v.flux += v.wave_vel * rb.dt_;
      } else {
        v.wave_vel += rb.delta_j_[i];
        v.flux += v.wave_vel;
      }

      // Langevin thermostat (Ornstein–Uhlenbeck on wave_vel, per-component).
      const bool langevin_active = rb.toggles.langevin &&
          ::ftd::site_matches_filter(rb.lattice_, i, rb.toggles.langevin_site_filter);
      if (langevin_active) {
        const double gamma = rb.toggles.langevin_gamma;
        const double T = rb.toggles.langevin_T;
        const double sigma = std::sqrt(2.0 * gamma * T);
        const double one_minus_gamma = 1.0 - gamma;
        v.wave_vel.x = one_minus_gamma * v.wave_vel.x + sigma * rng.thread_normal(tids);
        v.wave_vel.y = one_minus_gamma * v.wave_vel.y + sigma * rng.thread_normal(tids);
        v.wave_vel.z = one_minus_gamma * v.wave_vel.z + sigma * rng.thread_normal(tids);
      } else if (do_damping && should_damp) {
        double eff_damping = damping_factor;
        if (do_larmor && selective && rb.near_particle_[i]) {
          double a2 = rb.near_accel_[i] * rb.near_accel_[i];
          double larmor_mod = std::min(1.0, LARMOR_FLOOR + K_LARMOR * a2);
          eff_damping = 1.0 - DAMPING * larmor_mod;
        }
        v.flux *= eff_damping;
        v.wave_vel *= eff_damping;
      }

      // Genesis (single): divergence for polarity.
      if (do_genesis && v.state == 0 && v.density() > K_GENESIS) {
        double excess = v.density() - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_B);
        const std::uint64_t gseed =
            static_cast<std::uint64_t>(rb.toggles.langevin_seed);
        if (voxel_uniform(gseed, i, rb.tick_,
                          static_cast<std::uint64_t>(VoxelRng::GenesisManifest)) < p) {
          // Latent Heat of Manifestation: consume wave energy.
          v.wave_vel *= (1.0 - K_GENESIS_KINETIC_DRAIN);
          double jmag = v.flux.mag();
          if (jmag > K_GENESIS_FLUX_EPSILON)
            v.flux *= std::max(0.0, 1.0 - K_GENESIS / jmag);

          // ARCH-7b: divergence from pre-write snapshot (race-free).
          double div = ::ftd::divergence_from_flux_array(rb.flux_pre_write_, rb.lattice_, i);
          manifest_at(rb, v, div, rb.flux_pre_write_, rb.lattice_, i, gseed, rb.tick_, /*dual=*/false);
        }
      }
    }

    // Evaporation (shared single + dual): low TOTAL wave energy → return to void.
    constexpr double EVAP_ENERGY = K_B * K_B * EVAP_THRESHOLD;
    (void)EVAP_ENERGY;  // declared in original code; unused but preserved.
    double local_energy = v.flux.mag2() + v.wave_vel.mag2();
    {
      const auto& nbrs = rb.lattice_.neighbors_6(i);
      for (int n : nbrs)
        local_energy += rb.voxels_[n].flux.mag2() + rb.voxels_[n].wave_vel.mag2();
    }
    if ((do_genesis || do_evaporation) && v.state != 0 && !v.locked) {
      double evap_prob = std::exp(-local_energy / (K_B * K_B));
      const std::uint64_t gseed =
          static_cast<std::uint64_t>(rb.toggles.langevin_seed);
      if (voxel_uniform(gseed, i, rb.tick_,
                        static_cast<std::uint64_t>(VoxelRng::Evaporation)) < evap_prob * K_EVAP_RATE) {
        rb.set_state(i, 0);
        v.particle_id = -1;
        v.spin = 0;
        v.color = 0;
      }
    }
  }
}

void phase_write_assign_pending_ids(RenderBridge& rb) {
  // ARCH-7 (2026-04-25): sequential post-pass to assign deterministic
  // particle IDs in voxel-index order.
  const int N_total = static_cast<int>(rb.lattice_.total_sites());
  for (int i = 0; i < N_total; ++i) {
    if (rb.voxels_[i].particle_id == -2) {
      rb.voxels_[i].particle_id = rb.injector_.next_particle_id();
    }
  }
}

// Absorbing-boundary sponge — see render_bridge_phases.h. Quadratic ramp
// f(d) = (d/D)² over a shell of width D at every lattice face: f(0)=0 (Dirichlet
// wall), grading to 1 at d=D so the impedance change is gradual and reflects
// very little (~99.97% round-trip absorption at D=6). Damps the observable
// flux/wave_vel AND the dual L/R substrates (the observable is recomputed from
// them each tick, so damping only the observable would be overwritten). Byte-
// identical intent with the MockBridge JS sponge. O(N³) walk, but interior
// voxels (d≥D) short-circuit without writes.
void apply_absorbing_boundary(RenderBridge& rb) {
  const Lattice& lat = rb.lattice_;
  const int N = lat.size();
  const int Nm1 = N - 1;
  const int D = std::min(6, std::max(2, N / 4));
  const double invD = 1.0 / static_cast<double>(D);
  for (int z = 0; z < N; ++z) {
    const int dz = std::min(z, Nm1 - z);
    for (int y = 0; y < N; ++y) {
      const int dyz = std::min(std::min(y, Nm1 - y), dz);
      // NOTE: do NOT short-circuit the whole row on dyz>=D — the x-FACES of an
      // interior y/z row (dx<D) still need damping. Skipping them damped only the
      // y/z faces and left the x faces full, collapsing the volume to a slab.
      for (int x = 0; x < N; ++x) {
        const int d = std::min(std::min(x, Nm1 - x), dyz);
        if (d >= D) continue;  // per-voxel interior skip (symmetric on all 6 faces)
        const double r = d * invD;
        const double fd = r * r;
        Voxel& v = rb.voxels_[lat.index(x, y, z)];
        v.flux *= fd; v.wave_vel *= fd;
        v.flux_L *= fd; v.flux_R *= fd;
        v.wave_vel_L *= fd; v.wave_vel_R *= fd;
      }
    }
  }
}

}  // namespace ftd
