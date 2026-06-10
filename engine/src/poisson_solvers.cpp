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

  // NOTE: Red-Black SOR sweeps for an 18-point Laplacian MUST run sequentially
  // to remain deterministic and avoid read-write race conditions.
  // Proof: In a 3D grid, any edge-sharing neighbor (distance sqrt(2), e.g. x+1, y+1, z)
  // has coordinate sum (x+1)+(y+1)+z = x+y+z+2, which has the EXACT SAME color
  // (parity of x+y+z) as the center voxel. Because the 18-point stencil includes
  // 12 edge-sharing neighbors, a Red update reads from other Red voxels. Parallelizing
  // this sweep causes threads to concurrently read and write Red values, breaking
  // bit-exact golden determinism.
  for (int color = 0; color < 2; ++color) {
    for (int z = 1; z < Nm1; ++z) {
      for (int y = 1; y < Nm1; ++y) {
        const int parity_yz = (y + z) & 1;
        const int x_start = 1 + ((color ^ (parity_yz ^ 1)) & 1);
        int idx = z * LL + y * L + x_start;
        for (int x = x_start; x < Nm1; x += 2, idx += 2) {
          const double face_sum = phi[idx + o_xp] + phi[idx + o_xm]
                                + phi[idx + o_yp] + phi[idx + o_ym]
                                + phi[idx + o_zp] + phi[idx + o_zm];
          const double edge_sum = phi[idx + o_xpyp] + phi[idx + o_xpym]
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

    for (int z = 0; z < L; ++z) {
      const bool zEdge = (z == 0 || z == Nm1);
      for (int y = 0; y < L; ++y) {
        const bool yEdge = (y == 0 || y == Nm1);
        for (int x = 0; x < L; ++x) {
          const bool isInterior = !zEdge && !yEdge && x != 0 && x != Nm1;
          if (isInterior) continue;
          if (((x + y + z) & 1) != color) continue;
          const int idx = z * LL + y * L + x;
          const auto& face = lattice.neighbors_6(idx);
          const auto& edge = lattice.neighbors_12(idx);
          double face_sum = 0.0, edge_sum = 0.0;
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
  for (int i = 0; i < N; ++i) {
    const int iz = i % L;
    const int iy = (i / L) % L;
    const int ix = i / LL;
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

  for (int iter = 0; iter < sor_iters; ++iter) {
    sor_sweep_18pt(phi, sor_source, lattice, OMEGA);
  }

  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i) {
    phi_sum += phi[i];
  }
  const double phi_mean = phi_sum / N;
#pragma omp parallel for schedule(static)
  for (int i = 0; i < N; ++i) {
    phi[i] -= phi_mean;
  }

  for (int i = 0; i < N; ++i) {
    if (!exact_dual_gauss && state.state_at(i) != 0) continue;
    Vec3 grad_phi;
    const int iz = i % L;
    const int iy = (i / L) % L;
    const int ix = i / LL;
    if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
      grad_phi.x = (phi[i+LL] - phi[i-LL]) * 0.5;
      grad_phi.y = (phi[i+L]  - phi[i-L])  * 0.5;
      grad_phi.z = (phi[i+1]  - phi[i-1])  * 0.5;
    } else {
      const auto& n = lattice.neighbors_6(i);
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
    for (int i = 0; i < N; ++i)
      rho_sum += 0.5 * (voxels[i].flux.mag2() + voxels[i].wave_vel.mag2());
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
