/**
 * Transmutation phases — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R2.
 */

#include "ftd/transmutation_phases.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace ftd {

void weak_transmutation_cpu(RenderBridge& rb) {
  auto& voxels = rb.voxels_;
  const auto& lattice = rb.lattice_;
  const int N = static_cast<int>(lattice.total_sites());
  for (int i = 0; i < N; ++i) {
    auto& v = voxels[i];
    if (v.state == 0) continue;

    double stress = rb.toggles.dual_substrate
                      ? rb.compute_stress_left(i)
                      : rb.compute_stress(i);

    if (stress > WEAK_THRESHOLD) {
      double p = 1.0 - std::exp(-(stress - WEAK_THRESHOLD) / K_B);
      if (rb.uniform_(rb.rng_) < p) {
        v.state = -v.state;
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
  const auto& lattice = rb.lattice_;
  const int N = static_cast<int>(lattice.total_sites());
  for (int i = 0; i < N; ++i) {
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
  for (int i = 0; i < N; ++i) {
    auto& v = voxels[i];
    if (v.state != 0) continue;
    double jmag = v.flux.mag();
    if (jmag <= K_GENESIS) continue;

    double p = 1.0 - std::exp(-(jmag - K_GENESIS) / K_B);
    if (rb.uniform_(rb.rng_) >= p) continue;

    int partner = -1;
    for (int n : lattice.neighbors_6(i)) {
      if (voxels[n].state == 0) { partner = n; break; }
    }
    if (partner < 0) continue;

    int pid;
    pid = rb.next_particle_id_++;
    v.state = +1;
    v.particle_id = pid;
    v.pair_id = pid;

    auto& p2 = voxels[partner];
    p2.state = -1;
    p2.particle_id = rb.next_particle_id_++;
    p2.pair_id = pid;

    p2.flux = v.flux * -1.0;
  }
}

void triad_binding_cpu(RenderBridge& rb) {
  auto& voxels = rb.voxels_;
  const auto& lattice = rb.lattice_;
  const int N = static_cast<int>(lattice.total_sites());
  std::vector<int> particles;
  particles.reserve(64);
  for (int i = 0; i < N; ++i) {
    if (voxels[i].state != 0) particles.push_back(i);
  }

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

}  // namespace ftd
