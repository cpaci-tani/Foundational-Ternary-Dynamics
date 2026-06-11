/**
 * Poisson solvers — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R1.
 */

#include "ftd/poisson_solvers.h"
#include "ftd/constants.h"
#include <algorithm>
#include <cmath>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace ftd {

// ============================================================================
// SOR sweep helper — isotropic 18-point Poisson stencil with RED-BLACK ordering
// See original commentary in render_bridge.cpp (pre-refactor) for perf notes.
// ============================================================================
void sor_sweep_18pt(std::vector<double>& phi,
                    const std::vector<double>& source,
                    const Lattice& lattice,
                    double omega) {
  constexpr double INV3 = 1.0 / 3.0;
  constexpr double INV6 = 1.0 / 6.0;
  constexpr double INV4 = 1.0 / 4.0;
  const int L = lattice.size();
  const int LL = L * L;
  const int Nm1 = L - 1;

  const int o_xp = 1,  o_xm = -1;
  const int o_yp = L,  o_ym = -L;
  const int o_zp = LL, o_zm = -LL;
  const int o_xpyp = o_xp + o_yp, o_xpym = o_xp + o_ym;
  const int o_xmyp = o_xm + o_yp, o_xmym = o_xm + o_ym;
  const int o_xpzp = o_xp + o_zp, o_xpzm = o_xp + o_zm;
  const int o_xmzp = o_xm + o_zp, o_xmzm = o_xm + o_zm;
  const int o_ypzp = o_yp + o_zp, o_ypzm = o_yp + o_zm;
  const int o_ymzp = o_ym + o_zp, o_ymzm = o_ym + o_zm;

// NOTE: Standard 2-color Red-Black sweeps fail for the 18-point Laplacian because
  // the stencil includes 12 edge-sharing neighbors (radius-1 diagonals), which
  // create read-write races within the same Red/Black partition.
  // Instead, we use an 8-color (2x2x2) coloring scheme.
  //
  // DETERMINISM (golden gate): the 2x2x2 parity coloring is race-free ONLY for
  // INTERIOR cells, whose 18-point neighbours never leave the lattice. The
  // lattice has PERIODIC boundary conditions (lattice.h), and on an ODD lattice
  // the wrap maps coord Nm1 (even) → 0 (even): a face/edge neighbour of a
  // boundary cell can wrap to ANOTHER boundary cell of the SAME colour, so two
  // same-colour boundary cells become stencil-neighbours and racing — a genuine
  // read-write race, not a ULP reduction issue, that floats phi run-to-run.
  // Fix: update interior cells in PARALLEL (8-colour, race-free) and boundary
  // cells SEQUENTIALLY in lexicographic order per colour. This is bit-exact to a
  // fully-sequential lexicographic sweep because (a) within a colour every cell
  // reads only other-colour (frozen) neighbours, so interior update order is
  // irrelevant; (b) same-colour interior/boundary cells are never neighbours
  // (adjacency is an odd offset → different colour; only a wrap, i.e. two
  // boundary cells, yields a same-colour pair), so the interior/boundary split
  // does not change reads; (c) the seam boundary↔boundary same-colour pairs are
  // resolved in the same lexicographic order by the sequential boundary pass.
  // Boundary cells are O(L^2) (a small fraction for large L), so the parallel
  // interior sweep — the valuable hot loop — is preserved.
  for (int color = 0; color < 8; ++color) {
    int start_x = color & 1;
    int start_y = (color >> 1) & 1;
    int start_z = (color >> 2) & 1;

    // --- Interior cells: PARALLEL (fast path; never wraps → race-free) ---
#pragma omp parallel for schedule(static)
    for (int ix = start_x; ix < L; ix += 2) {
      if (ix == 0 || ix == Nm1) continue;  // x-face → boundary pass
      for (int iy = start_y; iy < L; iy += 2) {
        if (iy == 0 || iy == Nm1) continue;  // y-face → boundary pass
        for (int iz = start_z; iz < L; iz += 2) {
          if (iz == 0 || iz == Nm1) continue;  // z-face → boundary pass
          int idx = ix * LL + iy * L + iz;

          double face_sum = phi[idx + o_xp] + phi[idx + o_xm]
                          + phi[idx + o_yp] + phi[idx + o_ym]
                          + phi[idx + o_zp] + phi[idx + o_zm];
          double edge_sum = phi[idx + o_xpyp] + phi[idx + o_xpym]
                          + phi[idx + o_xmyp] + phi[idx + o_xmym]
                          + phi[idx + o_xpzp] + phi[idx + o_xpzm]
                          + phi[idx + o_xmzp] + phi[idx + o_xmzm]
                          + phi[idx + o_ypzp] + phi[idx + o_ypzm]
                          + phi[idx + o_ymzp] + phi[idx + o_ymzm];

          const double gs = (INV3 * face_sum + INV6 * edge_sum - source[idx]) * INV4;
          phi[idx] += omega * (gs - phi[idx]);
        }
      }
    }

    // --- Boundary cells: SEQUENTIAL lexicographic (slow path; wraps at seam) ---
    for (int ix = start_x; ix < L; ix += 2) {
      for (int iy = start_y; iy < L; iy += 2) {
        for (int iz = start_z; iz < L; iz += 2) {
          if (ix > 0 && ix < Nm1 && iy > 0 && iy < Nm1 && iz > 0 && iz < Nm1)
            continue;  // interior → already done in the parallel pass
          int idx = ix * LL + iy * L + iz;

          double face_sum = 0.0, edge_sum = 0.0;
          const auto& face = lattice.neighbors_6(ix, iy, iz);
          const auto& edge = lattice.neighbors_12(ix, iy, iz);
          for (int n : face) face_sum += phi[n];
          for (int n : edge) edge_sum += phi[n];

          const double gs = (INV3 * face_sum + INV6 * edge_sum - source[idx]) * INV4;
          phi[idx] += omega * (gs - phi[idx]);
        }
      }
    }
  }
}

// Local helper: central-difference divergence with lattice's periodic wrap.
// Mirrors RenderBridge::divergence_flux without requiring a bridge ref.
static inline double divergence_flux_at(const std::vector<Voxel>& voxels,
                                        const Lattice& lattice, int idx) {
  const auto& nbrs = lattice.neighbors_6(idx);
  double div = 0.0;
  div += (voxels[nbrs[0]].flux.x - voxels[nbrs[1]].flux.x) * 0.5;
  div += (voxels[nbrs[2]].flux.y - voxels[nbrs[3]].flux.y) * 0.5;
  div += (voxels[nbrs[4]].flux.z - voxels[nbrs[5]].flux.z) * 0.5;
  return div;
}

void gauss_project_cpu(std::vector<Voxel>& voxels,
                       const TernaryField& state,
                       std::vector<double>& phi,
                       std::vector<double>& sor_source,
                       const Lattice& lattice,
                       bool dual_substrate,
                       bool exact_dual_gauss,
                       double charge_coupling,
                       int sor_iters) {
  const int N = static_cast<int>(lattice.total_sites());
  const int L = lattice.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  constexpr double OMEGA = SOR_OMEGA;

  const double charge_sum = static_cast<double>(state.charge_sum());
  const double mean_charge = charge_sum / N;

#pragma omp parallel for schedule(static)
  for (int ix = 0; ix < L; ++ix) {
    for (int iy = 0; iy < L; ++iy) {
      for (int iz = 0; iz < L; ++iz) {
        const int i = ix * LL + iy * L + iz;
        double div;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          div = (voxels[i+LL].flux.x - voxels[i-LL].flux.x) * 0.5
              + (voxels[i+L].flux.y  - voxels[i-L].flux.y)  * 0.5
              + (voxels[i+1].flux.z  - voxels[i-1].flux.z)  * 0.5;
        } else {
          div = divergence_flux_at(voxels, lattice, i);
        }
        sor_source[i] = div - charge_coupling * (static_cast<double>(state.state_at(i)) - mean_charge);
      }
    }
  }

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi, sor_source, lattice, OMEGA);
  }

  // Sequential sum — DETERMINISM REQUIREMENT (golden gate). Float `+` under an
  // OpenMP reduction is not order-stable across threads, so a parallel reduction
  // floats phi_mean by ULPs run-to-run. The phi-mean shift is gauge-irrelevant
  // to grad(phi) (physics unchanged), but it leaks into absolute-phi audit
  // scalars (e.g. coulomb_pe) and breaks the bit-reproducible golden hash. This
  // is a single O(N) pass, dwarfed by the iterative SOR sweeps above, so the
  // cost of sequential summation is negligible. The 8-color SOR sweep stays
  // parallel (race-free, deterministic).
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i) {
    phi_sum += phi[i];
  }
  const double phi_mean = phi_sum / N;
#pragma omp parallel for schedule(static)
  for (int i = 0; i < N; ++i) {
    phi[i] -= phi_mean;
  }

#pragma omp parallel for schedule(static)
  for (int ix = 0; ix < L; ++ix) {
    for (int iy = 0; iy < L; ++iy) {
      for (int iz = 0; iz < L; ++iz) {
        const int i = ix * LL + iy * L + iz;
        if (!exact_dual_gauss && state.state_at(i) != 0) continue;
        Vec3 grad_phi;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          grad_phi.x = (phi[i+LL] - phi[i-LL]) * 0.5;
          grad_phi.y = (phi[i+L]  - phi[i-L])  * 0.5;
          grad_phi.z = (phi[i+1]  - phi[i-1])  * 0.5;
        } else {
          const auto& n = lattice.neighbors_6(ix, iy, iz);
          grad_phi.x = (phi[n[0]] - phi[n[1]]) * 0.5;
          grad_phi.y = (phi[n[2]] - phi[n[3]]) * 0.5;
          grad_phi.z = (phi[n[4]] - phi[n[5]]) * 0.5;
        }
        voxels[i].flux -= grad_phi;

        if (dual_substrate) {
          Vec3 half_corr = grad_phi * 0.5;
          voxels[i].flux_L -= half_corr;
          voxels[i].flux_R -= half_corr;
        }
      }
    }
  }
}

void solve_coulomb_poisson_cpu(const TernaryField& state,
                               std::vector<double>& phi_coulomb,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters) {
  const int N = static_cast<int>(lattice.total_sites());
  constexpr double OMEGA = SOR_OMEGA;

  double charge_sum = static_cast<double>(state.charge_sum());
  const double mean_charge = charge_sum / N;

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    sor_source[i] = -(static_cast<double>(state.state_at(i)) - mean_charge);
  }

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi_coulomb, sor_source, lattice, OMEGA);
  }

  // Sequential sum — DETERMINISM REQUIREMENT (golden gate); see note in
  // gauss_project_cpu. coulomb_pe in the energy audit reads absolute phi_coulomb
  // values, so a floated phi_mean here is the primary path that broke the hash.
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_coulomb[i];
  const double phi_mean = phi_sum / N;
#pragma omp parallel for
  for (int i = 0; i < N; ++i)
    phi_coulomb[i] -= phi_mean;
}

void solve_latency_poisson_cpu(std::vector<Voxel>& voxels,
                               const TernaryField& state,
                               std::vector<double>& phi_latency,
                               std::vector<double>& sor_source,
                               const Lattice& lattice,
                               int sor_iters,
                               bool include_field_energy) {
  const int N = static_cast<int>(lattice.total_sites());
  constexpr double OMEGA = SOR_OMEGA;
  constexpr double FOUR_PI_G = 4.0 * PI * G_N;

  // [IMPOSED] Gravitating density = particle rest mass (M_REST·|state|) plus, when
  // include_field_energy is set, the local field-energy density
  // ½(|J|²+|wave_vel|²) — the same ½|·|² convention as the energy audit
  // (diagnostics_compute.cpp). Motivated by GR sourcing gravity from the full
  // stress-energy so a flux-only configuration (e.g. a gravity wave) carries a
  // real potential; the coupling is imposed in the engine, not derived.
  double rho_sum = M_REST * static_cast<double>(state.manifested_count());
  if (include_field_energy) {
    // Sequential sum — DETERMINISM REQUIREMENT (golden gate); see note in
    // gauss_project_cpu. field_energy_sum sources the latency potential, so a
    // floated value here is not gauge-cancelled and reaches voxel latency.
    double field_energy_sum = 0.0;
    for (int i = 0; i < N; ++i) {
      field_energy_sum += 0.5 * (voxels[i].flux.mag2() + voxels[i].wave_vel.mag2());
    }
    rho_sum += field_energy_sum;
  }
  const double mean_rho = rho_sum / N;

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    double rho = M_REST * std::abs(state.state_at(i));
    if (include_field_energy)
      rho += 0.5 * (voxels[i].flux.mag2() + voxels[i].wave_vel.mag2());
    sor_source[i] = FOUR_PI_G * (rho - mean_rho);
  }

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi_latency, sor_source, lattice, OMEGA);
  }

  // Sequential sum — DETERMINISM REQUIREMENT (golden gate); see note in
  // gauss_project_cpu. voxel latency reads absolute phi_latency values.
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_latency[i];
  const double phi_mean = phi_sum / N;
#pragma omp parallel for
  for (int i = 0; i < N; ++i)
    phi_latency[i] -= phi_mean;

  for (int i = 0; i < N; ++i) {
    double phi_val = phi_latency[i];
    double abs_phi = std::abs(phi_val);
    double clamped = std::min(abs_phi, LATENCY_HORIZON_CLAMP);
    voxels[i].latency = std::sqrt(clamped);
  }
}

}  // namespace ftd
