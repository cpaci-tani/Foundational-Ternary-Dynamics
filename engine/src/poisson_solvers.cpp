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
                       std::vector<double>& phi,
                       std::vector<double>& sor_source,
                       const Lattice& lattice,
                       bool dual_substrate,
                       double charge_coupling) {
  const int N = static_cast<int>(lattice.total_sites());
  const int L = lattice.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;

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
    sor_source[i] = div - charge_coupling * static_cast<double>(voxels[i].state);
  }

  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    sor_sweep_18pt(phi, sor_source, lattice, OMEGA);
  }

#pragma omp parallel for schedule(static)
  for (int i = 0; i < N; ++i) {
    if (voxels[i].state != 0) continue;
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

void solve_coulomb_poisson_cpu(const std::vector<Voxel>& voxels,
                               std::vector<double>& phi_coulomb,
                               std::vector<double>& sor_source,
                               const Lattice& lattice) {
  const int N = static_cast<int>(lattice.total_sites());
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;

  double charge_sum = 0.0;
  for (int i = 0; i < N; ++i)
    charge_sum += static_cast<double>(voxels[i].state);
  const double mean_charge = charge_sum / N;

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    sor_source[i] = -(static_cast<double>(voxels[i].state) - mean_charge);
  }

  for (int iter = 0; iter < SOR_ITERS; ++iter) {
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
                               std::vector<double>& phi_latency,
                               std::vector<double>& sor_source,
                               const Lattice& lattice) {
  const int N = static_cast<int>(lattice.total_sites());
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;
  constexpr double FOUR_PI_G = 4.0 * PI * G_N;

  double mass_sum = 0.0;
  for (int i = 0; i < N; ++i)
    mass_sum += K_B * std::abs(voxels[i].state);
  const double mean_mass = mass_sum / N;

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    const double rho_mass = K_B * std::abs(voxels[i].state);
    sor_source[i] = FOUR_PI_G * (rho_mass - mean_mass);
  }

  for (int iter = 0; iter < SOR_ITERS; ++iter) {
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
