/**
 * Injection — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R5.
 *
 * ARCH-2-I (2026-04-25): migrated to use only RenderBridge's public API
 * (`backend()`, `voxels()`, `lattice()`, `injector()`, `gpu_engine_ptr()`).
 * No private-member access; the 6 friend declarations on inject_*_cpu can
 * now be dropped from RenderBridge.
 */

#include "ftd/injection.h"
#include "ftd/render_bridge.h"
#include "ftd/backend.h"
#include "ftd/constants.h"
#include <cmath>

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

void inject_flux_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val) {
#ifdef FTD_ENABLE_CUDA
  if (auto* gpu = rb.gpu_engine_ptr()) {
    // OPEN-5 fix (2026-04-25): GpuEngine has its own toggles (default
    // dual_substrate=true), and GpuEngine::inject_flux branches on it.
    // Sync just that one bit so callers who set rb.toggles.dual_substrate
    // BEFORE the first tick get the right behaviour. Wholesale toggle
    // copy is avoided here — it triggered an unrelated runtime issue,
    // tracked separately if it recurs.
    gpu->toggles.dual_substrate = rb.toggles.dual_substrate;
    // 2026-05-04 fix: must flush pending host-side voxel mutations to GPU
    // BEFORE the GPU inject. Without this, callers that mix host-side
    // mutations (`rb.voxels()[i].wave_vel = ...`) with `inject_flux` lose
    // their host edits when mark_gpu_dirty fires below — the GPU becomes
    // authoritative without ever seeing the host changes. test_maxwell
    // M1b/M5a, test_poynting PV-2 all hit this race; their wave_vel
    // initialisations were silently zeroed before tick 0.
    rb.backend().flush_host_mutations();
    gpu->inject_flux(x, y, z, flux_val);
    rb.backend().mark_gpu_dirty();
    return;
  }
#endif
  // CPU path: voxels() handles any GPU sync + dirty-marking automatically.
  auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
  v.flux = flux_val;
  if (rb.toggles.dual_substrate) {
    v.flux_L = flux_val * 0.5;
    v.flux_R = flux_val * 0.5;
  }
}

void inject_flux_add_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& flux_val) {
  // Mixed CPU/GPU pattern: read-modify-write on the host shadow with
  // automatic sync-down + dirty-up via voxels() accessor.
  auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
  v.flux = v.flux + flux_val;
  if (rb.toggles.dual_substrate) {
    const Vec3 half = flux_val * 0.5;
    v.flux_L = v.flux_L + half;
    v.flux_R = v.flux_R + half;
  }
}

void inject_wave_vel_add_cpu(RenderBridge& rb, int x, int y, int z, const Vec3& wv_val) {
  auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
  v.wave_vel = v.wave_vel + wv_val;
  if (rb.toggles.dual_substrate) {
    const Vec3 half = wv_val * 0.5;
    v.wave_vel_L = v.wave_vel_L + half;
    v.wave_vel_R = v.wave_vel_R + half;
  }
}

void inject_particle_cpu(RenderBridge& rb, int x, int y, int z, int8_t state,
                         const Vec3& flux_val, int8_t spin, int8_t color) {
#ifdef FTD_ENABLE_CUDA
  if (auto* gpu = rb.gpu_engine_ptr()) {
    gpu->toggles.dual_substrate = rb.toggles.dual_substrate;  // OPEN-5
    // Flush any pending host writes (e.g. a bg flux just copied via
    // voxels()[]= or copy_flux_and_wave_vel_for_coupling) BEFORE the
    // GPU-side inject. Without this, the next sync_to_host downloads
    // (zeros + injected charge) and clobbers the unflushed bg, leaving
    // the bg permanently lost. β-measurement seed-mute bug, 2026-04-26.
    rb.backend().flush_host_mutations();
    gpu->inject_particle(x, y, z, state, flux_val, spin, color);
    rb.backend().mark_gpu_dirty();
    return;
  }
#endif
  const int idx = rb.lattice().index(x, y, z);
  auto& v = rb.voxels()[idx];
  rb.set_state(idx, state);
  v.flux = flux_val;
  v.spin = spin;
  v.color = color;
  v.particle_id = rb.injector().next_particle_id();

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
  if (auto* gpu = rb.gpu_engine_ptr()) {
    gpu->toggles.dual_substrate = rb.toggles.dual_substrate;  // OPEN-5
    rb.backend().flush_host_mutations();   // see inject_particle_cpu rationale
    gpu->inject_wavepacket(cx, cy, cz, state, sigma, amplitude);
    rb.backend().mark_gpu_dirty();
    return;
  }
#endif
  // Grab references ONCE — voxels()/lattice() trigger backend dispatch each
  // call. The wavepacket loop touches O(radius^3) voxels; doing it through
  // a single reference is the common idiom.
  const auto& lattice = rb.lattice();
  auto& voxels = rb.voxels();

  int center = lattice.index(cx, cy, cz);
  auto& vc = voxels[center];
  rb.set_state(center, state);
  vc.particle_id = rb.injector().next_particle_id();

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
  const auto& lattice = rb.lattice();
  auto& voxels = rb.voxels();

  int id = rb.injector().next_pair_id();
  int idx = lattice.index(x, y, z);
  auto& v = voxels[idx];
  rb.set_state(idx, 1);
  v.flux = flux_val;
  v.pair_id = id;
  v.particle_id = rb.injector().next_particle_id();

  auto nbrs = lattice.neighbors_6(idx);
  int partner_idx = -1;
  for (int n : nbrs) {
    if (rb.state_at(n) == 0) { partner_idx = n; break; }
  }
  if (partner_idx < 0) return;

  auto& partner = voxels[partner_idx];
  rb.set_state(partner_idx, -1);
  partner.flux = flux_val * -1.0;
  partner.pair_id = id;
  partner.particle_id = rb.injector().next_particle_id();
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
  double max_j = 0.0;
  int site_count = 0;

  std::vector<double> radial_sum(20, 0.0);
  std::vector<int> radial_count(20, 0);

  for (int dx = -scan; dx <= scan; ++dx) {
    for (int dy = -scan; dy <= scan; ++dy) {
      for (int dz = -scan; dz <= scan; ++dz) {
        int x = ((cc.x + dx) % N + N) % N;
        int y = ((cc.y + dy) % N + N) % N;
        int z = ((cc.z + dz) % N + N) % N;
        int idx = lattice.index(x, y, z);

        double j_mag = voxels[idx].flux.mag();
        if (j_mag < threshold) continue;

        double j2 = j_mag * j_mag;
        sum_j2 += j2;
        sum_r2_j2 += (dx*dx + dy*dy + dz*dz) * j2;
        sum_rj2.x += dx * j2;
        sum_rj2.y += dy * j2;
        sum_rj2.z += dz * j2;
        max_j = std::max(max_j, j_mag);
        site_count++;

        int r_int = static_cast<int>(std::sqrt(static_cast<double>(dx*dx + dy*dy + dz*dz)));
        if (r_int >= 0 && r_int < 20) {
          radial_sum[r_int] += j_mag;
          radial_count[r_int]++;
        }
      }
    }
  }

  if (sum_j2 > EPSILON_FLUX_SQ) {
    prof.center_of_mass = Vec3(
      cc.x + sum_rj2.x / sum_j2,
      cc.y + sum_rj2.y / sum_j2,
      cc.z + sum_rj2.z / sum_j2
    );
    prof.effective_radius = std::sqrt(sum_r2_j2 / sum_j2);
  } else {
    prof.center_of_mass = Vec3(cc.x, cc.y, cc.z);
    prof.effective_radius = 0.0;
  }

  prof.total_energy = sum_j2;
  prof.peak_density = max_j;
  prof.site_count = site_count;

  for (int r = 0; r < 20; ++r) {
    prof.radial_profile[r] = (radial_count[r] > 0) ? radial_sum[r] / radial_count[r] : 0.0;
  }

  return prof;
}

}  // namespace ftd
