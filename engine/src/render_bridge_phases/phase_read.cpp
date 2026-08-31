/**
 * @file engine/src/render_bridge_phases/phase_read.cpp
 * @purpose Implementation of phase_read decomposition (Phase 4c, 2026-04-27).
 *
 * Extracted from render_bridge.cpp following the Phase 4a / 4b precedent
 * (phase_write.cpp, phase_forces.cpp) and the R1-R5 pattern. See ADR-0008.
 *
 * The original phase_read() was ~125 LOC of one OMP parallel-for that:
 *   - branches per-voxel between dual-substrate and single-substrate paths
 *   - within single-substrate, branches on toggles.bcc_stencil:
 *       FULL → interior 18-pt fast path (no modulo) + boundary slow path
 *       SC/FCC/BCC → laplacian_sublattice<> for all sites
 *   - adds state-flux coupling −g_c·∇s + g_c·∇×(s·v) (when toggles.coupling on;
 *     electric sign per lagrangian.h Term 2, amended 2026-07-18)
 *
 * The extraction preserves the parallel-for body BYTE-IDENTICAL. The golden
 * tick test (test_render_bridge_golden) hashes 100 ticks to the pinned
 * GOLDEN_HASH (current value lives in test_render_bridge_golden.cpp) and is
 * the strict gate on this refactor: any drift here is a physics bug.
 *
 * Why no per-branch split: the parallel-for would have to be re-walked once
 * per branch (cache-hostile and observable in microbenchmarks) or the
 * per-voxel branch decisions would need a scratch buffer (golden-gate
 * forbidden). The Phase 4a/4b precedent applies — extract orchestration,
 * keep loop body intact. There is no orchestration to extract here so this
 * TU exposes a single phase_read_main_loop(); the in-class phase_read()
 * orchestrator is now a one-line delegator.
 */

#include "ftd/render_bridge_phases.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/sublattice.h"
#include "ftd/field_operators.h"
#include "ftd/lorentz_bcc_time.h"
#include "ftd/lorentz_period2.h"
#include "ftd/parallel.h"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace ftd {

namespace {

// Dispersal never stores a transported value on the outer shell. For a
// strictly interior target whose stencil reaches that shell, substitute a
// target-local Sommerfeld value instead:
//     J_ghost = J_target - |offset| wave_target / (C_WAVE M_out).
// Keeping the closure local to the target is important for the 18-point Moore
// stencil. A single materialized face ghost is shared by tangentially adjacent
// targets and feeds one target's pseudo-velocity into another; that nonlocal
// feedback is the edge-growing mode caught by boundary_scenario_physics.
template <Vec3 Voxel::*Field, Vec3 Voxel::*Wave>
Vec3 dispersal_laplacian(const RenderBridge& rb, int x, int y, int z,
                         BccStencilMode mode) {
  const int L = rb.lattice().size();
  const int Nm1 = L - 1;
  if (x <= 0 || x >= Nm1 || y <= 0 || y >= Nm1 || z <= 0 || z >= Nm1) {
    return {};
  }

  static constexpr int faces[6][3] = {
      {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  static constexpr int edges[12][3] = {
      {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
      {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
      {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}};
  static constexpr int corners[8][3] = {
      {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
      {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}};

  const Lattice& lat = rb.lattice();
  const Voxel& target = rb.voxels()[static_cast<std::size_t>(
      lat.index(x, y, z))];
  const Vec3& field = target.*Field;
  const Vec3& wave = target.*Wave;
  const auto reaches_shell = [&](const int offset[3]) {
    const int nx = x + offset[0];
    const int ny = y + offset[1];
    const int nz = z + offset[2];
    return nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
        || nz == 0 || nz == Nm1;
  };

  // Normalize the impedance over the active outward part of the selected
  // stencil. Without this, a corner receives nearly three face-normal damping
  // impulses in one kick and the explicit map develops a growing corner mode.
  double outward_measure = 0.0;
  if (mode == BccStencilMode::SC || mode == BccStencilMode::FULL) {
    const double weight = mode == BccStencilMode::SC ? W_SC_FACE : 1.0 / 3.0;
    for (const auto& offset : faces) {
      if (reaches_shell(offset)) outward_measure += weight;
    }
  }
  if (mode == BccStencilMode::FCC || mode == BccStencilMode::FULL) {
    const double weight = mode == BccStencilMode::FCC ? W_FCC_EDGE : 1.0 / 6.0;
    for (const auto& offset : edges) {
      if (reaches_shell(offset)) outward_measure += weight * std::sqrt(2.0);
    }
  }
  if (mode == BccStencilMode::BCC) {
    for (const auto& offset : corners) {
      if (reaches_shell(offset)) {
        outward_measure += W_BCC_CORNER * std::sqrt(3.0);
      }
    }
  }
  const double inverse_measure = outward_measure > 0.0
      ? 1.0 / outward_measure : 0.0;
  const auto sample = [&](const int offset[3]) {
    const int nx = x + offset[0];
    const int ny = y + offset[1];
    const int nz = z + offset[2];
    if (nx == 0 || nx == Nm1 || ny == 0 || ny == Nm1
        || nz == 0 || nz == Nm1) {
      const double step = std::sqrt(static_cast<double>(
          offset[0] * offset[0] + offset[1] * offset[1]
          + offset[2] * offset[2]));
      return field - wave * (step * inverse_measure / C_WAVE);
    }
    return rb.voxels()[static_cast<std::size_t>(lat.index(nx, ny, nz))].*Field;
  };

  Vec3 sum;
  if (mode == BccStencilMode::SC) {
    for (const auto& offset : faces) sum += sample(offset);
    return sum * W_SC_FACE - field;
  }
  if (mode == BccStencilMode::FCC) {
    for (const auto& offset : edges) sum += sample(offset);
    return sum * W_FCC_EDGE - field;
  }
  if (mode == BccStencilMode::BCC) {
    for (const auto& offset : corners) sum += sample(offset);
    return sum * W_BCC_CORNER - field;
  }
  Vec3 face_sum;
  Vec3 edge_sum;
  for (const auto& offset : faces) face_sum += sample(offset);
  for (const auto& offset : edges) edge_sum += sample(offset);
  return face_sum * (1.0 / 3.0) + edge_sum * (1.0 / 6.0) - field * 4.0;
}

}  // namespace

void phase_read_main_loop(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  const int L = rb.lattice_.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  const bool do_wave = rb.toggles.wave_propagation;
  const bool do_coupling = rb.toggles.coupling;
  const bool dual = rb.toggles.dual_substrate;
  // FTD-0408 selected prototype: retain P4's one-Moore-shell dependency but
  // alternate the wave kick coefficient over the microscopic two-tick cell.
  // The exact Floquet pole is sin^2(theta)=M/13+3M^2/676; it is stable on
  // 0<=M<=16/3 and has no q^4 term in theta^2.  The negative odd-tick kick
  // and period-two clock are selected inputs.  Default OFF is bit-neutral.
  // FTD-0411 selected two-domain prototype.  The literal normalized BCC
  // clock has unwanted scalar cube-root branches, so this path uses its
  // stable period-two IR localization.  It matches c^2=1/7 and cancels q^4,
  // but deliberately does not claim exact BCC temporal dynamics at q^6.
  const double cw2 = rb.toggles.lorentz_bcc_time_floquet
      ? lorentz_bcc_time_kappa(rb.tick_)
      : (rb.toggles.lorentz_period2_floquet
          ? lorentz_period2_kappa(rb.tick_)
          : C_WAVE * C_WAVE);

  // FTD-0271: de Broglie internal clock — Klein-Gordon rest-mass term −ω₀²·J
  // applied at manifested (state≠0) voxels. delta_j is acceleration (∂²J/∂t²),
  // so the leapfrog integrator turns −ω₀²·J into the KG dispersion ω²=c²k²+ω₀²
  // and a static cluster's flux oscillates at ω₀. The clock is [IMPOSED]
  // (native flux is massless, A0). Strictly additive and gated below; with the
  // toggle OFF this is a dead branch, so the golden hash is unaffected.
  const bool do_db_clock = rb.toggles.de_broglie_clock;
  const bool do_db_clock_coulomb = do_db_clock && rb.toggles.db_clock_coulomb;
  const double omega0 = rb.toggles.omega0;
  const double omega0_sq = rb.toggles.omega0 * rb.toggles.omega0;

  const TernaryField& state = rb.ternary_field();
  const Lattice& lat = rb.lattice_;

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
    ftd::parallel_for(0, L, [&](int _lo, int _hi) {
    for (int ix = _lo; ix < _hi; ++ix) {
      for (int iy = 0; iy < L; ++iy) {
        for (int iz = 0; iz < L; ++iz) {
          const int i = ix * LL + iy * L + iz;
          rb.delta_j_L_[i] = {};
          rb.delta_j_R_[i] = {};

          if (do_wave) {
            Vec3 lap_L, lap_R;
            const bool dispersal_near_shell =
                rb.toggles.flux_boundary == FluxBoundaryMode::Dispersal
                && (ix <= 1 || ix >= Nm1 - 1
                    || iy <= 1 || iy >= Nm1 - 1
                    || iz <= 1 || iz >= Nm1 - 1);
            if (dispersal_near_shell) {
              lap_L = dispersal_laplacian<&Voxel::flux_L, &Voxel::wave_vel_L>(
                  rb, ix, iy, iz, BccStencilMode::FULL);
              lap_R = dispersal_laplacian<&Voxel::flux_R, &Voxel::wave_vel_R>(
                  rb, ix, iy, iz, BccStencilMode::FULL);
            } else if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
              // Interior fast path: precomputed offsets, zero modulo operations.
              // Neighbor offsets: ±1=±z, ±L=±y, ±LL=±x (matches lattice coord convention).
              const Vec3 fL = (rb.voxels_[i+1].flux_L  + rb.voxels_[i-1].flux_L
                             + rb.voxels_[i+L].flux_L  + rb.voxels_[i-L].flux_L
                             + rb.voxels_[i+LL].flux_L + rb.voxels_[i-LL].flux_L) * INV3;
              const Vec3 fR = (rb.voxels_[i+1].flux_R  + rb.voxels_[i-1].flux_R
                             + rb.voxels_[i+L].flux_R  + rb.voxels_[i-L].flux_R
                             + rb.voxels_[i+LL].flux_R + rb.voxels_[i-LL].flux_R) * INV3;
              const Vec3 eL = (rb.voxels_[i+1+L].flux_L  + rb.voxels_[i+1-L].flux_L
                             + rb.voxels_[i-1+L].flux_L  + rb.voxels_[i-1-L].flux_L
                             + rb.voxels_[i+1+LL].flux_L + rb.voxels_[i+1-LL].flux_L
                             + rb.voxels_[i-1+LL].flux_L + rb.voxels_[i-1-LL].flux_L
                             + rb.voxels_[i+L+LL].flux_L + rb.voxels_[i+L-LL].flux_L
                             + rb.voxels_[i-L+LL].flux_L + rb.voxels_[i-L-LL].flux_L) * INV6;
              const Vec3 eR = (rb.voxels_[i+1+L].flux_R  + rb.voxels_[i+1-L].flux_R
                             + rb.voxels_[i-1+L].flux_R  + rb.voxels_[i-1-L].flux_R
                             + rb.voxels_[i+1+LL].flux_R + rb.voxels_[i+1-LL].flux_R
                             + rb.voxels_[i-1+LL].flux_R + rb.voxels_[i-1-LL].flux_R
                             + rb.voxels_[i+L+LL].flux_R + rb.voxels_[i+L-LL].flux_R
                             + rb.voxels_[i-L+LL].flux_R + rb.voxels_[i-L-LL].flux_R) * INV6;
              lap_L = fL + eL - rb.voxels_[i].flux_L * 4.0;
              lap_R = fR + eR - rb.voxels_[i].flux_R * 4.0;
            } else {
              // Boundary slow path: modular wrapping via lattice neighbor tables.
              lap_L = ::ftd::laplacian_field<&Voxel::flux_L>(rb.voxels_, rb.lattice_, i);
              lap_R = ::ftd::laplacian_field<&Voxel::flux_R>(rb.voxels_, rb.lattice_, i);
            }
            rb.delta_j_L_[i] = lap_L * cw2;
            rb.delta_j_R_[i] = lap_R * cw2;
          }

          // Coupling source: split equally between L and R substrates.
          // Electric part is −g_c·∇s (Term 2 sign amendment 2026-07-18): the
          // drive points OUTWARD at a +1 charge, sourcing div J toward the
          // Gauss target instead of against it (pre-fix live equilibrium was
          // f = −0.095 wrong-signed; see test_gauss_law_fidelity.cpp).
          if (do_coupling) {
            Vec3 grad_s = ::ftd::gradient_state_op(state, lat, ix, iy, iz) * (G_C * 0.5);
            Vec3 curl_sv = ::ftd::curl_state_velocity_op(state, rb.voxels_, lat, ix, iy, iz) * (G_C * 0.5);
            rb.delta_j_L_[i] += curl_sv - grad_s;
            rb.delta_j_R_[i] += curl_sv - grad_s;
          }

          // FTD-0271/0281: de Broglie clock. This imposed K_B-tied clock is a
          // matter-site rest term. The FTD-0281 diagnostic is the operator-spectroscopy
          // version: all clocked field sites feel V=-phi_C, matching
          // omega_eff^2 = omega0^2 + 2*omega0*V from FTD-0278.
          if (do_db_clock_coulomb) {
            const double omega_eff_sq = omega0_sq - 2.0 * omega0 * rb.phi_coulomb_[i];
            rb.delta_j_L_[i] -= rb.voxels_[i].flux_L * omega_eff_sq;
            rb.delta_j_R_[i] -= rb.voxels_[i].flux_R * omega_eff_sq;
          } else if (do_db_clock && rb.voxels_[i].state != 0) {
            rb.delta_j_L_[i] -= rb.voxels_[i].flux_L * omega0_sq;
            rb.delta_j_R_[i] -= rb.voxels_[i].flux_R * omega0_sq;
          }
        }
      }
    }
    });
  } else {
    // Single-substrate: inline Laplacian with the same interior/boundary split.
    // Cluster A (FTD-0093): when toggles.bcc_stencil != FULL, dispatch to the
    // selected sub-stencil via laplacian_sublattice<>. The interior fast path
    // is retained ONLY for FULL mode; the SC/FCC/BCC paths use the slow path
    // unconditionally — the experimental campaign trades raw throughput for
    // an obviously-correct sub-stencil projection. Toggle validation guarantees
    // dual_substrate==false in this branch when bcc_stencil != FULL.
    const BccStencilMode stencil_mode = rb.toggles.bcc_stencil;
    ftd::parallel_for(0, L, [&](int _lo, int _hi) {
    for (int ix = _lo; ix < _hi; ++ix) {
      for (int iy = 0; iy < L; ++iy) {
        for (int iz = 0; iz < L; ++iz) {
          const int i = ix * LL + iy * L + iz;
          rb.delta_j_[i] = {};

          if (do_wave) {
            Vec3 lap;
            const bool dispersal_near_shell =
                rb.toggles.flux_boundary == FluxBoundaryMode::Dispersal
                && (ix <= 1 || ix >= Nm1 - 1
                    || iy <= 1 || iy >= Nm1 - 1
                    || iz <= 1 || iz >= Nm1 - 1);
            if (dispersal_near_shell) {
              lap = dispersal_laplacian<&Voxel::flux, &Voxel::wave_vel>(
                  rb, ix, iy, iz, stencil_mode);
            } else if (stencil_mode == BccStencilMode::FULL) {
              if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
                // Interior fast path (FULL stencil only)
                const Vec3 f = (rb.voxels_[i+1].flux  + rb.voxels_[i-1].flux
                              + rb.voxels_[i+L].flux  + rb.voxels_[i-L].flux
                              + rb.voxels_[i+LL].flux + rb.voxels_[i-LL].flux) * INV3;
                const Vec3 e = (rb.voxels_[i+1+L].flux  + rb.voxels_[i+1-L].flux
                              + rb.voxels_[i-1+L].flux  + rb.voxels_[i-1-L].flux
                              + rb.voxels_[i+1+LL].flux + rb.voxels_[i+1-LL].flux
                              + rb.voxels_[i-1+LL].flux + rb.voxels_[i-1-LL].flux
                              + rb.voxels_[i+L+LL].flux + rb.voxels_[i+L-LL].flux
                              + rb.voxels_[i-L+LL].flux + rb.voxels_[i-L-LL].flux) * INV6;
                lap = f + e - rb.voxels_[i].flux * 4.0;
              } else {
                // Boundary slow path (FULL stencil)
                lap = rb.laplacian_flux(i);
              }
            } else {
              // Sublattice projection (SC, FCC, or BCC). Slow path for all sites.
              lap = ::ftd::laplacian_sublattice<&Voxel::flux>(stencil_mode,
                                                              rb.voxels_, rb.lattice_, i);
            }
            rb.delta_j_[i] = lap * cw2;
          }

          if (do_coupling) {
            // Electric source −g_c·∇s (Term 2 sign amendment 2026-07-18; see
            // the dual-substrate branch above and test_gauss_law_fidelity.cpp).
            rb.delta_j_[i] -= ::ftd::gradient_state_op(state, lat, ix, iy, iz) * G_C;
            rb.delta_j_[i] += ::ftd::curl_state_velocity_op(state, rb.voxels_, lat, ix, iy, iz) * G_C;
          }

          // FTD-0271/0281: de Broglie clock. See the dual-substrate branch
          // above for the FTD-0281 all-site Coulomb-coupled diagnostic.
          if (do_db_clock_coulomb) {
            const double omega_eff_sq = omega0_sq - 2.0 * omega0 * rb.phi_coulomb_[i];
            rb.delta_j_[i] -= rb.voxels_[i].flux * omega_eff_sq;
          } else if (do_db_clock && rb.voxels_[i].state != 0) {
            rb.delta_j_[i] -= rb.voxels_[i].flux * omega0_sq;
          }
        }
      }
    }
    });
  }
}

}  // namespace ftd
