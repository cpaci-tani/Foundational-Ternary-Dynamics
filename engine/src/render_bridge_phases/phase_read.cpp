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
 *   - adds state-flux coupling g_c·∇s + g_c·∇×(s·v) (when toggles.coupling on)
 *
 * The extraction preserves the parallel-for body BYTE-IDENTICAL. The golden
 * tick test (test_render_bridge_golden) hashes 100 ticks to
 * 0xcd957b601d47868a and is the strict gate on this refactor: any drift
 * here is a physics bug.
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

#ifdef _OPENMP
#include <omp.h>
#endif

namespace ftd {

void phase_read_main_loop(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  const int L = rb.lattice_.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  const bool do_wave = rb.toggles.wave_propagation;
  const bool do_coupling = rb.toggles.coupling;
  const bool dual = rb.toggles.dual_substrate;
  const double cw2 = C_WAVE * C_WAVE;

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
#pragma omp parallel for schedule(static)
    for (int ix = 0; ix < L; ++ix) {
      for (int iy = 0; iy < L; ++iy) {
        for (int iz = 0; iz < L; ++iz) {
          const int i = ix * LL + iy * L + iz;
          rb.delta_j_L_[i] = {};
          rb.delta_j_R_[i] = {};

          if (do_wave) {
            Vec3 lap_L, lap_R;
            if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
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

          // Coupling source: split equally between L and R substrates
          if (do_coupling) {
            Vec3 grad_s = ::ftd::gradient_state_op(state, lat, ix, iy, iz) * (G_C * 0.5);
            Vec3 curl_sv = ::ftd::curl_state_velocity_op(state, rb.voxels_, lat, ix, iy, iz) * (G_C * 0.5);
            rb.delta_j_L_[i] += grad_s + curl_sv;
            rb.delta_j_R_[i] += grad_s + curl_sv;
          }
        }
      }
    }
  } else {
    // Single-substrate: inline Laplacian with the same interior/boundary split.
    // Cluster A (FTD-0093): when toggles.bcc_stencil != FULL, dispatch to the
    // selected sub-stencil via laplacian_sublattice<>. The interior fast path
    // is retained ONLY for FULL mode; the SC/FCC/BCC paths use the slow path
    // unconditionally — the experimental campaign trades raw throughput for
    // an obviously-correct sub-stencil projection. Toggle validation guarantees
    // dual_substrate==false in this branch when bcc_stencil != FULL.
    const BccStencilMode stencil_mode = rb.toggles.bcc_stencil;
#pragma omp parallel for schedule(static)
    for (int ix = 0; ix < L; ++ix) {
      for (int iy = 0; iy < L; ++iy) {
        for (int iz = 0; iz < L; ++iz) {
          const int i = ix * LL + iy * L + iz;
          rb.delta_j_[i] = {};

          if (do_wave) {
            Vec3 lap;
            if (stencil_mode == BccStencilMode::FULL) {
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
            rb.delta_j_[i] += ::ftd::gradient_state_op(state, lat, ix, iy, iz) * G_C;
            rb.delta_j_[i] += ::ftd::curl_state_velocity_op(state, rb.voxels_, lat, ix, iy, iz) * G_C;
          }
        }
      }
    }
  }
}

}  // namespace ftd
