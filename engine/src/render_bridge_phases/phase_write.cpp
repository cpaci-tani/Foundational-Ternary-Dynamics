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
 * tick test (test_render_bridge_golden) hashes 100 ticks to the pinned
 * GOLDEN_HASH (current value lives in test_render_bridge_golden.cpp) and is
 * the strict gate on this refactor: any drift here is a physics bug.
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
#include "ftd/proper_time_rate.h"
#include "ftd/voxel_rng.h"
#include "ftd/parallel.h"
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

  // ---- Loop 1: Leapfrog integration, Langevin OU, and Damping ----
  ftd::parallel_for(0, N, [&](int _lo, int _hi) {
  for (int i = _lo; i < _hi; ++i) {
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
      if (rb.toggles.verlet_wave_integrator) {
        // E1 (FTD-0337): velocity-Verlet KDK part 1 — half-kick + drift.
        // The second half-kick (wave_vel += ½·dt·ΔJ' at the post-drift
        // field) is applied by tick() after phase_write. Default OFF ⇒
        // dead branch ⇒ golden hash untouched.
        v.wave_vel_L += rb.delta_j_L_[i] * (0.5 * rb.dt_);
        v.wave_vel_R += rb.delta_j_R_[i] * (0.5 * rb.dt_);
        v.flux_L += v.wave_vel_L * rb.dt_;
        v.flux_R += v.wave_vel_R * rb.dt_;
      } else if (rb.toggles.symplectic_leapfrog) {
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
    } else {
      // ---- Single-substrate leapfrog integration (non-dual path) ----
      if (rb.toggles.verlet_wave_integrator) {
        // E1 (FTD-0337): velocity-Verlet KDK part 1 — half-kick + drift.
        // Second half-kick applied by tick() after phase_write.
        v.wave_vel += rb.delta_j_[i] * (0.5 * rb.dt_);
        v.flux += v.wave_vel * rb.dt_;
      } else if (rb.toggles.symplectic_leapfrog) {
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
        // FDT-consistent discrete OU: Var_stationary = sigma^2/(1-(1-gamma)^2) = T exactly
        // (was sqrt(2*gamma*T), the Euler-Maruyama form, biased to T/(1-gamma/2)).
        const double sigma = std::sqrt(gamma * (2.0 - gamma) * T);
        const double one_minus_gamma = 1.0 - gamma;
        const std::uint64_t gseed = static_cast<std::uint64_t>(rb.toggles.langevin_seed);
        const double nx = ::ftd::voxel_normal(gseed, i, rb.tick_,
            static_cast<std::uint64_t>(::ftd::VoxelRng::LangevinNoiseX));
        const double ny = ::ftd::voxel_normal(gseed, i, rb.tick_,
            static_cast<std::uint64_t>(::ftd::VoxelRng::LangevinNoiseY));
        const double nz = ::ftd::voxel_normal(gseed, i, rb.tick_,
            static_cast<std::uint64_t>(::ftd::VoxelRng::LangevinNoiseZ));
        v.wave_vel.x = one_minus_gamma * v.wave_vel.x + sigma * nx;
        v.wave_vel.y = one_minus_gamma * v.wave_vel.y + sigma * ny;
        v.wave_vel.z = one_minus_gamma * v.wave_vel.z + sigma * nz;
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
    }
  }
  });

  // ---- Snapshot the updated flux field (post-write) to rb.flux_pre_write_ ----
  // (which is now acting as post-write snapshot) to avoid cross-thread races.
  if (do_genesis) {
    rb.flux_pre_write_.resize(N);
    ftd::parallel_for(0, N, [&](int _lo, int _hi) {
    for (int i = _lo; i < _hi; ++i) {
      rb.flux_pre_write_[i] = rb.voxels_[i].flux;
    }
    });
  }

  // FTD-0267 observation-only telemetry: reset per-tick genesis/evaporation
  // event counters before the genesis/evaporation loop. Pure counters — they
  // touch no physics state, RNG draw, or control flow (golden-gated).
  rb.genesis_events_this_tick_ = 0;
  rb.evaporation_events_this_tick_ = 0;

  // Effective genesis constants. Defaults (override <=0, use_temperature=false)
  // reproduce the compile-time K_GENESIS / K_MANIFEST byte-for-byte ⇒ golden-safe.
  // kg = genesis threshold; km = manifestation probability-ramp scale. The
  // research overrides let a campaign test K_GENESIS = K_B (drop the N_c) and a
  // ramp tied to temperature (km = langevin_T) instead of the electron mass.
  // Single-substrate genesis only.
  const double kg = (rb.genesis_threshold_override > 0.0)
                      ? rb.genesis_threshold_override : K_GENESIS;
  const double km = rb.manifest_use_temperature
                      ? std::max(rb.toggles.langevin_T, 1e-12)
                      : ((rb.manifest_scale_override > 0.0)
                           ? rb.manifest_scale_override : K_MANIFEST);

  // ---- Loop 2: Genesis and Evaporation ----
  // SEQUENTIAL — DETERMINISM REQUIREMENT (golden gate). This loop carries a
  // genuine cross-thread read-write hazard: evaporation reads neighbour
  // voxels_[n].flux / wave_vel *live* (below), while genesis on another thread
  // writes a firing voxel's OWN flux / wave_vel (the K_GENESIS drain). The
  // evaporation outcome therefore depends on which neighbours fired genesis
  // first, so no race-free PARALLEL ordering reproduces the canonical
  // linear-index result — only a fixed sequential order does. The RNG here is
  // the stateless index-keyed voxel_uniform(gseed, i, tick, …) (no per-thread
  // state), and real work fires only on the rare supercritical / manifested
  // voxels, so the sequential cost is negligible next to the parallel wave /
  // forces / SOR phases. Do NOT re-parallelise without an order-independent
  // formulation (e.g. all decisions read a frozen pre-genesis snapshot — which
  // would change physics relative to the live-read reference).
  for (int i = 0; i < N; ++i) {
    auto &v = rb.voxels_[i];
    const std::uint64_t gseed =
        static_cast<std::uint64_t>(rb.toggles.langevin_seed);

    if (dual) {
      // Genesis (dual): chirality density for polarity.
      if (do_genesis && v.state == 0 && v.flux.mag2() > K_GENESIS * K_GENESIS) {
        double dens = std::sqrt(v.flux.mag2());
        double excess = dens - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_MANIFEST);
        if (voxel_uniform(gseed, i, rb.tick_,
                          static_cast<std::uint64_t>(VoxelRng::GenesisManifest)) < p) {
          ftd::atomic_inc(rb.genesis_events_this_tick_);  // FTD-0267 telemetry (observation only)
          double chi = v.chirality_density();
          manifest_at(rb, v, chi, rb.flux_pre_write_, rb.lattice_, i, gseed, rb.tick_, /*dual=*/true);
        }
      }
    } else {
      // Genesis (single): divergence for polarity.
      if (do_genesis && v.state == 0 && v.flux.mag2() > kg * kg) {
        double dens = std::sqrt(v.flux.mag2());
        double excess = dens - kg;
        double p = 1.0 - std::exp(-excess / km);
        if (voxel_uniform(gseed, i, rb.tick_,
                          static_cast<std::uint64_t>(VoxelRng::GenesisManifest)) < p) {
          ftd::atomic_inc(rb.genesis_events_this_tick_);  // FTD-0267 telemetry (observation only)
          // Latent Heat of Manifestation: consume wave energy. FTD-0276: the
          // drain fraction is a runtime toggle (default 0.5 = legacy constant).
          v.wave_vel *= (1.0 - rb.toggles.kinetic_drain);
          double jmag = dens;
          if (jmag > K_GENESIS_FLUX_EPSILON)
            v.flux *= std::max(0.0, 1.0 - kg / jmag);

          // divergence from the post-write snapshot (race-free).
          double div = ::ftd::divergence_from_flux_array(rb.flux_pre_write_, rb.lattice_, i);
          manifest_at(rb, v, div, rb.flux_pre_write_, rb.lattice_, i, gseed, rb.tick_, /*dual=*/false);
        }
      }
    }

    // Evaporation (shared single + dual): low TOTAL wave energy → return to void.
    if ((do_genesis || do_evaporation) && v.state != 0 && !v.locked) {
      double local_energy = v.flux.mag2() + v.wave_vel.mag2();
      {
        const auto& nbrs = rb.lattice_.neighbors_6(i);
        for (int n : nbrs)
          local_energy += rb.voxels_[n].flux.mag2() + rb.voxels_[n].wave_vel.mag2();
      }
      double evap_prob = std::exp(-local_energy / (K_MANIFEST * K_MANIFEST));
      // Proper-time hazard (2026-07-19 amendment; owner ruling on
      // PREREG_TWO_CLOCK_CONSISTENCY_v1 Outcome A): the decay clock integrates
      // the SAME dτ the proper-time accumulator defines (ftd/proper_time_rate.h)
      // — a metastable population in a latency well decays slower by √(1−L²)
      // at rest and by the SR factor when moving. At L=0, v=0 the factor is
      // exactly 1 (bit-identical to the pre-amendment rule). The RNG draw and
      // stream are unchanged; only the acceptance threshold scales.
      const double dtau = proper_time_rate(v.latency, v.speed() * v.speed());
      if (voxel_uniform(gseed, i, rb.tick_,
                        static_cast<std::uint64_t>(VoxelRng::Evaporation)) < evap_prob * K_EVAP_RATE * dtau) {
        ftd::atomic_inc(rb.evaporation_events_this_tick_);  // FTD-0267 telemetry (observation only)
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
// ── Shared boundary-shell primitives (revision 2.3 dedup) ──────────────────
// The three boundary passes below touch the same six flux fields (observable
// + dual L/R substrates); before this dedup any new substrate field had to be
// added in three places. Field ORDER inside the helpers matches the original
// blocks exactly (bit-identical requirement — pinned by
// test_boundary_modes_golden).

// Scale all six flux fields of one voxel by s.
static inline void scale_flux_fields(Voxel& v, double s) {
  v.flux *= s; v.wave_vel *= s;
  v.flux_L *= s; v.flux_R *= s;
  v.wave_vel_L *= s; v.wave_vel_R *= s;
}

// Copy all six flux fields from src into dst.
static inline void copy_flux_fields(Voxel& dst, const Voxel& src) {
  dst.flux = src.flux;             dst.wave_vel = src.wave_vel;
  dst.flux_L = src.flux_L;         dst.flux_R = src.flux_R;
  dst.wave_vel_L = src.wave_vel_L; dst.wave_vel_R = src.wave_vel_R;
}

// Visit every voxel of the one-layer boundary shell (all six faces) in
// z-outer/y-mid/x-inner order — the traversal both one-layer passes share.
template <typename Fn>
static inline void for_each_shell_voxel(RenderBridge& rb, Fn&& fn) {
  // Public accessor, not rb.lattice_: this file-static template is NOT a
  // RenderBridge friend (only the boundary passes that call it are).
  const int N = rb.lattice().size();
  const int Nm1 = N - 1;
  for (int z = 0; z < N; ++z)
  for (int y = 0; y < N; ++y)
  for (int x = 0; x < N; ++x) {
    if (x > 0 && x < Nm1 && y > 0 && y < Nm1 && z > 0 && z < Nm1) continue;  // interior
    fn(x, y, z, Nm1);
  }
}

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
      // (This D-deep ramp iterates differently from the one-layer shell walk,
      // so it keeps its own loop and shares only the field-scaling helper.)
      for (int x = 0; x < N; ++x) {
        const int d = std::min(std::min(x, Nm1 - x), dyz);
        if (d >= D) continue;  // per-voxel interior skip (symmetric on all 6 faces)
        const double r = d * invD;
        const double fd = r * r;
        scale_flux_fields(rb.voxels_[lat.index(x, y, z)], fd);
      }
    }
  }
}

// Reflective flux boundary (FluxBoundaryMode::Reflective) — Neumann mirror.
// Copy the first interior layer into the boundary shell each tick so ∂_n J = 0
// at every face: a perfect free reflector (a closed cavity). Energy is conserved
// inside the box. NOTE: a closed cavity does NOT drain an injection-driven
// runaway — that is by design (the "secondary" mode). One-layer overwrite,
// applied AFTER the last flux writers like the sponge. Gated → golden-neutral.
void apply_reflective_flux_boundary(RenderBridge& rb) {
  const Lattice& lat = rb.lattice_;
  if (lat.size() < 3) return;
  for_each_shell_voxel(rb, [&](int x, int y, int z, int Nm1) {
    const int ix = (x == 0) ? 1 : (x == Nm1 ? Nm1 - 1 : x);
    const int iy = (y == 0) ? 1 : (y == Nm1 ? Nm1 - 1 : y);
    const int iz = (z == 0) ? 1 : (z == Nm1 ? Nm1 - 1 : z);
    copy_flux_fields(rb.voxels_[lat.index(x, y, z)],
                     rb.voxels_[lat.index(ix, iy, iz)]);
  });
}

// Dispersal flux boundary (FluxBoundaryMode::Dispersal) — single-cell radiating
// sink. The outermost layer is the interface to the void: the field that reaches
// it propagates out at ~wave speed c and is removed from the box ("disappears
// into the void and is removed from memory"). This is ONE sharp cell, NOT the
// graduated quadratic sponge. It is the open boundary that drains an injection-
// driven runaway toward a bounded steady state. Applied AFTER the last flux
// writers. Gated → golden-neutral.
void apply_dispersal_flux_boundary(RenderBridge& rb) {
  const Lattice& lat = rb.lattice_;
  // Fraction of the outer layer that propagates into the void this tick (c·dt).
  const double keep = 1.0 - C_SPEED;
  for_each_shell_voxel(rb, [&](int x, int y, int z, int) {
    scale_flux_fields(rb.voxels_[lat.index(x, y, z)], keep);
  });
}

}  // namespace ftd
