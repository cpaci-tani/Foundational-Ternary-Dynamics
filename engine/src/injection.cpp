/**
 * Injection — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R5.
 * CUDA branches preserved verbatim (the gpu_ pointer and flags live on rb).
 */

#include "ftd/injection.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include <cmath>

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

void inject_flux_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val) {
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.gpu_->inject_flux(x, y, z, flux_val);
    rb.gpu_dirty_ = true;
    return;
  }
#endif
  auto& v = rb.voxels_[rb.lattice_.index(x, y, z)];
  v.flux = flux_val;
  if (rb.toggles.dual_substrate) {
    v.flux_L = flux_val * 0.5;
    v.flux_R = flux_val * 0.5;
  }
}

void inject_flux_add_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val) {
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.gpu_sync_to_host();
  }
#endif
  auto& v = rb.voxels_[rb.lattice_.index(x, y, z)];
  v.flux = v.flux + flux_val;
  if (rb.toggles.dual_substrate) {
    const Vec3 half = flux_val * 0.5;
    v.flux_L = v.flux_L + half;
    v.flux_R = v.flux_R + half;
  }
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.host_mutated_ = true;
  }
#endif
}

void inject_wave_vel_add_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& wv_val) {
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.gpu_sync_to_host();
  }
#endif
  auto& v = rb.voxels_[rb.lattice_.index(x, y, z)];
  v.wave_vel = v.wave_vel + wv_val;
  if (rb.toggles.dual_substrate) {
    const Vec3 half = wv_val * 0.5;
    v.wave_vel_L = v.wave_vel_L + half;
    v.wave_vel_R = v.wave_vel_R + half;
  }
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.host_mutated_ = true;
  }
#endif
}

void inject_particle_cpu(RenderBridge& rb, int x, int y, int z, int8_t state,
                         const Vec3& flux_val, int8_t spin, int8_t color) {
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.gpu_->inject_particle(x, y, z, state, flux_val, spin, color);
    rb.gpu_dirty_ = true;
    return;
  }
#endif
  auto& v = rb.voxels_[rb.lattice_.index(x, y, z)];
  v.state = state;
  v.flux = flux_val;
  v.spin = spin;
  v.color = color;
  v.particle_id = rb.next_particle_id_++;

  if (rb.toggles.dual_substrate) {
    double frac_major = (1.0 + DELTA_APPROX) * 0.5;
    double frac_minor = (1.0 - DELTA_APPROX) * 0.5;
    if (state > 0) {
      v.flux_L = flux_val * frac_major;
      v.flux_R = flux_val * frac_minor;
    } else {
      v.flux_L = flux_val * frac_minor;
      v.flux_R = flux_val * frac_major;
    }
  }
}

void inject_wavepacket_cpu(RenderBridge& rb, int cx, int cy, int cz, int8_t state,
                           double sigma, double amplitude) {
#ifdef FTD_ENABLE_CUDA
  if (rb.use_gpu_) {
    rb.gpu_->inject_wavepacket(cx, cy, cz, state, sigma, amplitude);
    rb.gpu_dirty_ = true;
    return;
  }
#endif
  const auto& lattice = rb.lattice_;
  auto& voxels = rb.voxels_;

  int center = lattice.index(cx, cy, cz);
  auto& vc = voxels[center];
  vc.state = state;
  vc.particle_id = rb.next_particle_id_++;

  int radius = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
  double norm_sum = 0.0;
  int N = lattice.size();

  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > GAUSSIAN_CUTOFF_SIGMA * sigma) continue;
        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        norm_sum += g * g;
      }
    }
  }

  double scale = (norm_sum > EPSILON_FLUX_SQ) ? amplitude / std::sqrt(norm_sum) : 0.0;

  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > GAUSSIAN_CUTOFF_SIGMA * sigma) continue;

        int x = ((cx + dx) % N + N) % N;
        int y = ((cy + dy) % N + N) % N;
        int z = ((cz + dz) % N + N) % N;
        int idx = lattice.index(x, y, z);

        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        double mag = scale * g;
        Vec3 flux_inc(mag * dx / r, mag * dy / r, mag * dz / r);
        voxels[idx].flux += flux_inc;

        if (rb.toggles.dual_substrate) {
          double frac_major = (1.0 + DELTA_APPROX) * 0.5;
          double frac_minor = (1.0 - DELTA_APPROX) * 0.5;
          if (state > 0) {
            voxels[idx].flux_L += flux_inc * frac_major;
            voxels[idx].flux_R += flux_inc * frac_minor;
          } else {
            voxels[idx].flux_L += flux_inc * frac_minor;
            voxels[idx].flux_R += flux_inc * frac_major;
          }
        }
      }
    }
  }
}

void create_entangled_pair_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val) {
  const auto& lattice = rb.lattice_;
  auto& voxels = rb.voxels_;

  int id = rb.next_pair_id_++;
  int idx = lattice.index(x, y, z);
  auto& v = voxels[idx];
  v.state = 1;
  v.flux = flux_val;
  v.pair_id = id;
  v.particle_id = rb.next_particle_id_++;

  auto nbrs = lattice.neighbors_6(idx);
  int partner_idx = -1;
  for (int n : nbrs) {
    if (voxels[n].state == 0) { partner_idx = n; break; }
  }
  if (partner_idx < 0) return;

  auto& partner = voxels[partner_idx];
  partner.state = -1;
  partner.flux = flux_val * -1.0;
  partner.pair_id = id;
  partner.particle_id = rb.next_particle_id_++;
}

AggregateProfile compute_aggregate_profile(const RenderBridge& rb, int center_idx, double threshold) {
  AggregateProfile prof;
  const auto& lattice = rb.lattice();
  const auto& voxels = rb.voxels();
  auto cc = lattice.coord(center_idx);
  int N = lattice.size();
  int scan = 20;

  double sum_j2 = 0.0;
  double sum_r2_j2 = 0.0;
  Vec3 sum_rj2;
  int radial_count[20] = {};
  double radial_sum[20] = {};

  for (int dx = -scan; dx <= scan; ++dx) {
    for (int dy = -scan; dy <= scan; ++dy) {
      for (int dz = -scan; dz <= scan; ++dz) {
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        int ri = static_cast<int>(std::round(r));
        if (ri < 1 || ri > 20) continue;

        int x = ((cc.x + dx) % N + N) % N;
        int y = ((cc.y + dy) % N + N) % N;
        int z = ((cc.z + dz) % N + N) % N;
        int idx = lattice.index(x, y, z);
        double j2 = voxels[idx].flux.mag2();
        double jmag = std::sqrt(j2);

        if (jmag > threshold) prof.site_count++;
        if (jmag > prof.peak_density) prof.peak_density = jmag;

        sum_j2 += j2;
        sum_r2_j2 += r2 * j2;
        sum_rj2.x += (cc.x + dx) * j2;
        sum_rj2.y += (cc.y + dy) * j2;
        sum_rj2.z += (cc.z + dz) * j2;

        radial_sum[ri - 1] += jmag;
        radial_count[ri - 1]++;
      }
    }
  }

  double j2_center = voxels[center_idx].flux.mag2();
  sum_j2 += j2_center;

  prof.total_energy = sum_j2;
  prof.effective_radius = (sum_j2 > EPSILON_FLUX_SQ) ? std::sqrt(sum_r2_j2 / sum_j2) : 0.0;

  if (sum_j2 > EPSILON_FLUX_SQ) {
    prof.center_of_mass = Vec3(sum_rj2.x / sum_j2, sum_rj2.y / sum_j2, sum_rj2.z / sum_j2);
  } else {
    auto c = lattice.coord(center_idx);
    prof.center_of_mass = Vec3(c.x, c.y, c.z);
  }

  for (int i = 0; i < 20; ++i) {
    prof.radial_profile[i] = (radial_count[i] > 0) ? radial_sum[i] / radial_count[i] : 0.0;
  }

  return prof;
}

}  // namespace ftd
