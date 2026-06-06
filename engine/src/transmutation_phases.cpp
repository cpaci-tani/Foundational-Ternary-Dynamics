/**
 * Transmutation phases — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R2.
 */

#include "ftd/transmutation_phases.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel_rng.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdint>

namespace ftd {

void weak_transmutation_cpu(RenderBridge& rb) {
  auto& voxels = rb.voxels_;
  const auto& active = rb.ordered_active_indices();
  const std::uint64_t gseed = static_cast<std::uint64_t>(rb.toggles.langevin_seed);
  for (int i : active) {
    auto& v = voxels[i];
    if (v.state == 0) continue;

    double stress = rb.toggles.dual_substrate
                      ? rb.compute_stress_left(i)
                      : rb.compute_stress(i);

    if (stress > WEAK_THRESHOLD) {
      double p = 1.0 - std::exp(-(stress - WEAK_THRESHOLD) / K_MANIFEST);
      if (voxel_uniform(gseed, i, rb.tick_,
                        static_cast<std::uint64_t>(VoxelRng::WeakTransmutation)) < p) {
        rb.set_state(i, static_cast<int8_t>(-v.state));
        if (rb.toggles.dual_substrate) {
          std::swap(v.flux_L, v.flux_R);
          std::swap(v.wave_vel_L, v.wave_vel_R);
        }
      }
    }
  }
}

void accumulate_proper_time(RenderBridge& rb) {
  auto& voxels = rb.voxels_;
  const auto& active = rb.ordered_active_indices();
  for (int i : active) {
    auto& v = voxels[i];
    if (v.state == 0) continue;
    double L = v.latency;
    double f = 1.0 - L * L;
    if (f <= 0.0) continue;
    double v2 = v.speed() * v.speed();
    double arg = f * f - v2;
    if (arg > 0.0)
      v.tau += std::sqrt(arg) / std::sqrt(f);
  }
}

void pair_production_cpu(RenderBridge& rb) {
  auto& voxels = rb.voxels_;
  const auto& lattice = rb.lattice_;
  const int N = static_cast<int>(lattice.total_sites());
  const std::uint64_t gseed = static_cast<std::uint64_t>(rb.toggles.langevin_seed);
  for (int i = 0; i < N; ++i) {
    auto& v = voxels[i];
    if (v.state != 0) continue;
    double jmag = v.flux.mag();
    if (jmag <= K_GENESIS) continue;

    double p = 1.0 - std::exp(-(jmag - K_GENESIS) / K_MANIFEST);
    if (voxel_uniform(gseed, i, rb.tick_,
                      static_cast<std::uint64_t>(VoxelRng::PairProduction)) >= p) continue;

    // Geometric Pair Production: find the major axis of the flux vector
    int dx = 0, dy = 0, dz = 0;
    double fx = std::abs(v.flux.x), fy = std::abs(v.flux.y), fz = std::abs(v.flux.z);
    if (fx >= fy && fx >= fz) dx = (v.flux.x > 0) ? 1 : -1;
    else if (fy >= fx && fy >= fz) dy = (v.flux.y > 0) ? 1 : -1;
    else dz = (v.flux.z > 0) ? 1 : -1;

    auto coord = lattice.coord(i);
    int nx = coord.x + dx;
    int ny = coord.y + dy;
    int nz = coord.z + dz;
    // Periodic boundary
    int L = lattice.size();
    if (nx < 0) nx += L; else if (nx >= L) nx -= L;
    if (ny < 0) ny += L; else if (ny >= L) ny -= L;
    if (nz < 0) nz += L; else if (nz >= L) nz -= L;

    int partner = lattice.index(nx, ny, nz);
    if (voxels[partner].state != 0) continue; // Partner space must be empty

    // Latent Heat of Manifestation: consume wave energy
    v.wave_vel *= 0.5;
    voxels[partner].wave_vel *= 0.5;
    v.flux *= std::max(0.0, 1.0 - K_GENESIS / jmag); // Consume potential energy

    int pid = rb.injector_.next_particle_id();
    // The +1 charge should be pushed downstream by the external field, and the -1 upstream.
    // The vector `d` points downstream. Therefore, the partner is downstream.
    // To oppose the external field (Vacuum Polarization), the dipole must point UPSTREAM.
    // So the downstream particle must be +1, and the upstream particle -1.
    // `v` is upstream, `partner` is downstream.
    rb.set_state(i, -1);
    v.particle_id = pid;
    v.pair_id = pid;

    auto& p2 = voxels[partner];
    rb.set_state(partner, +1);
    p2.particle_id = rb.injector_.next_particle_id();
    p2.pair_id = pid;

    p2.flux = v.flux * -1.0;
  }
}

void triad_binding_cpu(RenderBridge& rb) {
  auto& voxels = rb.voxels_;
  const auto& lattice = rb.lattice_;
  std::vector<int> particles;
  const auto& active = rb.ordered_active_indices();
  particles.assign(active.begin(), active.end());

  auto coord_dist = [&](int a, int b) {
    auto ca = lattice.coord(a), cb = lattice.coord(b);
    double dx = ca.x - cb.x, dy = ca.y - cb.y, dz = ca.z - cb.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
  };

  const int M = static_cast<int>(particles.size());
  for (int a = 0; a < M; ++a) {
    auto& va = voxels[particles[a]];
    if (va.locked) continue;
    for (int b = a + 1; b < M; ++b) {
      auto& vb = voxels[particles[b]];
      if (vb.locked || vb.state != va.state) continue;
      double rAB = coord_dist(particles[a], particles[b]);
      if (rAB > TRIAD_RADIUS) continue;
      for (int c = b + 1; c < M; ++c) {
        auto& vc = voxels[particles[c]];
        if (vc.locked || vc.state != va.state) continue;
        double rAC = coord_dist(particles[a], particles[c]);
        double rBC = coord_dist(particles[b], particles[c]);
        if (rAC > TRIAD_RADIUS || rBC > TRIAD_RADIUS) continue;
        double rmin = std::min({rAB, rAC, rBC});
        double rmax = std::max({rAB, rAC, rBC});
        if (rmax < 1e-9) continue;
        if (rmin / rmax < TRIAD_RATIO_THRESHOLD) continue;
        va.locked = true;
        vb.locked = true;
        vc.locked = true;
        break;
      }
    }
  }
}

namespace {

inline int wrap(int c, int L) {
    return ((c % L) + L) % L;
}

inline int idx(int x, int y, int z, int L) {
    return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

} // namespace

void relax_su2_links_cpu(RenderBridge& rb, double dt, double beta) {
  auto& links_x = rb.su2_links_x_;
  auto& links_y = rb.su2_links_y_;
  auto& links_z = rb.su2_links_z_;
  const int L = rb.lattice_.size();

  // Helper lambda to fetch a link safely
  auto fetch_link = [&](int s, int dir) -> SU2Link {
    if (dir == 0) return links_x[s];
    if (dir == 1) return links_y[s];
    return links_z[s];
  };

  // Helper to multiply two SU2 links
  auto multiply_su2 = [](const SU2Link& u, const SU2Link& v) -> SU2Link {
    std::complex<double> new_a = u.a * v.a - u.b * std::conj(v.b);
    std::complex<double> new_b = u.a * v.b + u.b * std::conj(v.a);
    return SU2Link(new_a, new_b);
  };

  // Helper to get adjoint of SU2 link
  auto adjoint_su2 = [](const SU2Link& u) -> SU2Link {
    return SU2Link(std::conj(u.a), -u.b);
  };

#pragma omp parallel for
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        int site = idx(x, y, z, L);

        // Compute staples for each direction mu = 0, 1, 2
        for (int mu = 0; mu < 3; ++mu) {
          SU2Link staple(std::complex<double>(0.0, 0.0), std::complex<double>(0.0, 0.0));
          bool first_staple = true;

          for (int nu = 0; nu < 3; ++nu) {
            if (mu == nu) continue;

            // Positive plaquette contribution:
            // U_nu(x + mu) * U_mu^\dagger(x + nu) * U_nu^\dagger(x)
            int site_xpmu = 0;
            int site_xpnu = 0;
            if (mu == 0)      site_xpmu = idx(x + 1, y, z, L);
            else if (mu == 1) site_xpmu = idx(x, y + 1, z, L);
            else              site_xpmu = idx(x, y, z + 1, L);

            if (nu == 0)      site_xpnu = idx(x + 1, y, z, L);
            else if (nu == 1) site_xpnu = idx(x, y + 1, z, L);
            else              site_xpnu = idx(x, y, z + 1, L);

            SU2Link u_nu_xpmu = fetch_link(site_xpmu, nu);
            SU2Link u_mu_xpnu = fetch_link(site_xpnu, mu);
            SU2Link u_nu_x    = fetch_link(site, nu);

            SU2Link term1 = multiply_su2(u_nu_xpmu, multiply_su2(adjoint_su2(u_mu_xpnu), adjoint_su2(u_nu_x)));

            // Negative plaquette contribution:
            // U_nu^\dagger(x + mu - nu) * U_mu^\dagger(x - nu) * U_nu(x - nu)
            int site_xmnust = 0;
            int site_xpmu_mnu = 0;
            if (nu == 0) {
              site_xmnust = idx(x - 1, y, z, L);
              if (mu == 0)      site_xpmu_mnu = idx(x, y, z, L);
              else if (mu == 1) site_xpmu_mnu = idx(x - 1, y + 1, z, L);
              else              site_xpmu_mnu = idx(x - 1, y, z + 1, L);
            } else if (nu == 1) {
              site_xmnust = idx(x, y - 1, z, L);
              if (mu == 0)      site_xpmu_mnu = idx(x + 1, y - 1, z, L);
              else if (mu == 1) site_xpmu_mnu = idx(x, y, z, L);
              else              site_xpmu_mnu = idx(x, y - 1, z + 1, L);
            } else {
              site_xmnust = idx(x, y, z - 1, L);
              if (mu == 0)      site_xpmu_mnu = idx(x + 1, y, z - 1, L);
              else if (mu == 1) site_xpmu_mnu = idx(x, y + 1, z - 1, L);
              else              site_xpmu_mnu = idx(x, y, z, L);
            }

            SU2Link u_nu_xpmu_mnu = fetch_link(site_xpmu_mnu, nu);
            SU2Link u_mu_xmnu     = fetch_link(site_xmnust, mu);
            SU2Link u_nu_xmnu     = fetch_link(site_xmnust, nu);

            SU2Link term2 = multiply_su2(adjoint_su2(u_nu_xpmu_mnu), multiply_su2(adjoint_su2(u_mu_xmnu), u_nu_xmnu));

            if (first_staple) {
              staple = SU2Link(term1.a + term2.a, term1.b + term2.b);
              first_staple = false;
            } else {
              staple = SU2Link(staple.a + term1.a + term2.a, staple.b + term1.b + term2.b);
            }
          }

          // Local minimization update: U_new = Proj[ U_old + dt * beta * staple^\dagger ]
          SU2Link u_old = fetch_link(site, mu);
          SU2Link staple_adj = adjoint_su2(staple);

          SU2Link u_new(u_old.a + staple_adj.a * (dt * beta), u_old.b + staple_adj.b * (dt * beta));
          u_new.normalize();

          // Write back
          if (mu == 0)      links_x[site] = u_new;
          else if (mu == 1) links_y[site] = u_new;
          else              links_z[site] = u_new;
        }
      }
    }
  }
}

void relax_su3_links_cpu(RenderBridge& rb, double dt, double beta) {
  auto& links_x = rb.su3_links_x_;
  auto& links_y = rb.su3_links_y_;
  auto& links_z = rb.su3_links_z_;
  const int L = rb.lattice_.size();

  // Helper lambda to fetch a link safely
  auto fetch_link = [&](int s, int dir) -> SU3Link {
    if (dir == 0) return links_x[s];
    if (dir == 1) return links_y[s];
    return links_z[s];
  };

  // Helper to multiply two SU3 links
  auto multiply_su3 = [](const SU3Link& u, const SU3Link& v) -> SU3Link {
    SU3Link res;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        std::complex<double> sum(0.0, 0.0);
        for (int k = 0; k < 3; ++k) {
          sum += u.m[i][k] * v.m[k][j];
        }
        res.m[i][j] = sum;
      }
    }
    return res;
  };

  // Helper to get adjoint of SU3 link
  auto adjoint_su3 = [](const SU3Link& u) -> SU3Link {
    SU3Link res;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        res.m[i][j] = std::conj(u.m[j][i]);
      }
    }
    return res;
  };

  // Helper to normalize SU3 matrix using Gram-Schmidt
  auto normalize_su3 = [](SU3Link& u) {
    // Row 0 normalization
    double r0_mag = std::sqrt(std::norm(u.m[0][0]) + std::norm(u.m[0][1]) + std::norm(u.m[0][2]));
    if (r0_mag > 1e-12) {
      u.m[0][0] /= r0_mag;
      u.m[0][1] /= r0_mag;
      u.m[0][2] /= r0_mag;
    }

    // Row 1 orthogonalization against Row 0
    std::complex<double> r1_dot_r0 = u.m[1][0] * std::conj(u.m[0][0]) +
                                    u.m[1][1] * std::conj(u.m[0][1]) +
                                    u.m[1][2] * std::conj(u.m[0][2]);
    u.m[1][0] -= r1_dot_r0 * u.m[0][0];
    u.m[1][1] -= r1_dot_r0 * u.m[0][1];
    u.m[1][2] -= r1_dot_r0 * u.m[0][2];

    double r1_mag = std::sqrt(std::norm(u.m[1][0]) + std::norm(u.m[1][1]) + std::norm(u.m[1][2]));
    if (r1_mag > 1e-12) {
      u.m[1][0] /= r1_mag;
      u.m[1][1] /= r1_mag;
      u.m[1][2] /= r1_mag;
    }

    // Row 2 = (Row 0 x Row 1)*
    u.m[2][0] = std::conj(u.m[0][1] * u.m[1][2] - u.m[0][2] * u.m[1][1]);
    u.m[2][1] = std::conj(u.m[0][2] * u.m[1][0] - u.m[0][0] * u.m[1][2]);
    u.m[2][2] = std::conj(u.m[0][0] * u.m[1][1] - u.m[0][1] * u.m[1][0]);
  };

#pragma omp parallel for
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        int site = idx(x, y, z, L);

        // Compute staples for each direction mu = 0, 1, 2
        for (int mu = 0; mu < 3; ++mu) {
          SU3Link staple;
          // Set to all zeros initially
          for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) staple.m[r][c] = 0.0;
          }

          for (int nu = 0; nu < 3; ++nu) {
            if (mu == nu) continue;

            // Positive plaquette contribution:
            // U_nu(x + mu) * U_mu^\dagger(x + nu) * U_nu^\dagger(x)
            int site_xpmu = 0;
            int site_xpnu = 0;
            if (mu == 0)      site_xpmu = idx(x + 1, y, z, L);
            else if (mu == 1) site_xpmu = idx(x, y + 1, z, L);
            else              site_xpmu = idx(x, y, z + 1, L);

            if (nu == 0)      site_xpnu = idx(x + 1, y, z, L);
            else if (nu == 1) site_xpnu = idx(x, y + 1, z, L);
            else              site_xpnu = idx(x, y, z + 1, L);

            SU3Link u_nu_xpmu = fetch_link(site_xpmu, nu);
            SU3Link u_mu_xpnu = fetch_link(site_xpnu, mu);
            SU3Link u_nu_x    = fetch_link(site, nu);

            SU3Link term1 = multiply_su3(u_nu_xpmu, multiply_su3(adjoint_su3(u_mu_xpnu), adjoint_su3(u_nu_x)));

            // Negative plaquette contribution:
            // U_nu^\dagger(x + mu - nu) * U_mu^\dagger(x - nu) * U_nu(x - nu)
            int site_xmnust = 0;
            int site_xpmu_mnu = 0;
            if (nu == 0) {
              site_xmnust = idx(x - 1, y, z, L);
              if (mu == 0)      site_xpmu_mnu = idx(x, y, z, L);
              else if (mu == 1) site_xpmu_mnu = idx(x - 1, y + 1, z, L);
              else              site_xpmu_mnu = idx(x - 1, y, z + 1, L);
            } else if (nu == 1) {
              site_xmnust = idx(x, y - 1, z, L);
              if (mu == 0)      site_xpmu_mnu = idx(x + 1, y - 1, z, L);
              else if (mu == 1) site_xpmu_mnu = idx(x, y, z, L);
              else              site_xpmu_mnu = idx(x, y - 1, z + 1, L);
            } else {
              site_xmnust = idx(x, y, z - 1, L);
              if (mu == 0)      site_xpmu_mnu = idx(x + 1, y, z - 1, L);
              else if (mu == 1) site_xpmu_mnu = idx(x, y + 1, z - 1, L);
              else              site_xpmu_mnu = idx(x, y, z, L);
            }

            SU3Link u_nu_xpmu_mnu = fetch_link(site_xpmu_mnu, nu);
            SU3Link u_mu_xmnu     = fetch_link(site_xmnust, mu);
            SU3Link u_nu_xmnu     = fetch_link(site_xmnust, nu);

            SU3Link term2 = multiply_su3(adjoint_su3(u_nu_xpmu_mnu), multiply_su3(adjoint_su3(u_mu_xmnu), u_nu_xmnu));

            for (int r = 0; r < 3; ++r) {
              for (int c = 0; c < 3; ++c) {
                staple.m[r][c] += term1.m[r][c] + term2.m[r][c];
              }
            }
          }

          // Local minimization update: U_new = Proj[ U_old + dt * beta * staple^\dagger ]
          SU3Link u_old = fetch_link(site, mu);
          SU3Link staple_adj = adjoint_su3(staple);

          SU3Link u_new;
          for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
              u_new.m[r][c] = u_old.m[r][c] + staple_adj.m[r][c] * (dt * beta);
            }
          }
          normalize_su3(u_new);

          // Write back
          if (mu == 0)      links_x[site] = u_new;
          else if (mu == 1) links_y[site] = u_new;
          else              links_z[site] = u_new;
        }
      }
    }
  }
}

}  // namespace ftd
