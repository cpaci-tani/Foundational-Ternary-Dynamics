/**
 * @file engine/src/render_bridge_phases/phase_movement.cpp
 * @purpose Implementation of phase_movement decomposition (Phase 4c, 2026-04-27).
 *
 * Extracted from render_bridge.cpp following the Phase 4a / 4b precedent
 * (phase_write.cpp, phase_forces.cpp) and the R1-R5 pattern. See ADR-0008.
 *
 * The original phase_movement() was ~110 LOC of one SEQUENTIAL per-voxel
 * loop that:
 *   - accumulates velocity·dt into per-voxel remainder
 *   - on |remainder| ≥ 1, takes an integer-jump on each axis
 *   - dispatches collision against the target voxel:
 *       * void target → move (carry self-field flux up to K_B; dual L/R too)
 *       * same-sign target → elastic bounce (axis-flip velocity)
 *       * opposite-sign target → annihilation (zero both, distribute each
 *         flux burst over the corresponding 6-neighbor shell)
 *   - sets moved_[target] to prevent double-processing this tick
 *
 * The extraction preserves the per-voxel loop body BYTE-IDENTICAL. The
 * golden tick test (test_render_bridge_golden) hashes 100 ticks to
 * 0xcd957b601d47868a and is the strict gate on this refactor: any drift
 * here is a physics bug.
 *
 * Why no per-pass split: each iteration mutates two voxels (the moving
 * particle and its target). Subsequent iterations read those mutations via
 * the moved_ guard (prevents the just-arrived particle from being processed
 * again as a source) and via direct voxel reads (the target's new state
 * matters for the next collision). Splitting the loop into drift /
 * annihilation / compact passes either records (i, target) decisions in a
 * scratch buffer to be applied later (different observable order — consider
 * three particles in a chain where particle B moves into C's site and is
 * then visited as A's target) or runs multiple sequential passes that each
 * re-read mutated state (different physics). Both break the golden gate.
 * Mirror Phase 4a/4b: extract one main-loop function with the full body
 * verbatim. There is no orchestration to extract here.
 */

#include "ftd/render_bridge_phases.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel_rng.h"
#include <algorithm>
#include <random>
#include <vector>
#include <numeric>

namespace ftd {

void phase_movement_main_loop(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  std::fill(rb.moved_.begin(), rb.moved_.end(), 0);

  if (rb.toggles.symmetric_movement_order) {
    if (static_cast<int>(rb.movement_indices_.size()) != N) {
      rb.movement_indices_.resize(N);
    }
    auto& indices = rb.movement_indices_;
    std::iota(indices.begin(), indices.end(), 0);
    std::uint64_t seed = (static_cast<std::uint64_t>(rb.toggles.langevin_seed) << 32) ^ static_cast<std::uint64_t>(rb.tick_);
    unsigned int seed_32 = static_cast<unsigned int>(seed ^ (seed >> 32));
    std::mt19937 rng(seed_32);
    std::shuffle(indices.begin(), indices.end(), rng);

    for (int i : indices) {
      auto &v = rb.voxels_[i];
      if (v.state == 0 || v.locked || rb.moved_[i]) continue;

      v.remainder += v.velocity * rb.dt_;

      auto c = rb.lattice_.coord(i);
      int dx = 0, dy = 0, dz = 0;

      int axes[3] = {0, 1, 2};
      double r = voxel_uniform(rb.toggles.langevin_seed, i, rb.tick_, static_cast<std::uint64_t>(VoxelRng::MovementOrder));
      int perm = static_cast<int>(r * 6.0);
      if (perm < 0) perm = 0;
      else if (perm > 5) perm = 5;

      switch (perm) {
        case 0: axes[0] = 0; axes[1] = 1; axes[2] = 2; break; // xyz
        case 1: axes[0] = 0; axes[1] = 2; axes[2] = 1; break; // xzy
        case 2: axes[0] = 1; axes[1] = 0; axes[2] = 2; break; // yxz
        case 3: axes[0] = 1; axes[1] = 2; axes[2] = 0; break; // yzx
        case 4: axes[0] = 2; axes[1] = 0; axes[2] = 1; break; // zxy
        case 5: axes[0] = 2; axes[1] = 1; axes[2] = 0; break; // zyx
      }

      for (int axis_idx = 0; axis_idx < 3; ++axis_idx) {
        int axis = axes[axis_idx];
        if (axis == 0) {
          if (v.remainder.x >= 1.0) { dx = 1; v.remainder.x -= 1.0; }
          else if (v.remainder.x <= -1.0) { dx = -1; v.remainder.x += 1.0; }
        } else if (axis == 1) {
          if (v.remainder.y >= 1.0) { dy = 1; v.remainder.y -= 1.0; }
          else if (v.remainder.y <= -1.0) { dy = -1; v.remainder.y += 1.0; }
        } else {
          if (v.remainder.z >= 1.0) { dz = 1; v.remainder.z -= 1.0; }
          else if (v.remainder.z <= -1.0) { dz = -1; v.remainder.z += 1.0; }
        }
      }

      if (dx == 0 && dy == 0 && dz == 0) continue;

      int target = rb.lattice_.index(c.x + dx, c.y + dy, c.z + dz);
      auto &t = rb.voxels_[target];

      if (t.state == 0) {
        // Move: transfer particle to target
        const int8_t moving_state = v.state;
        rb.set_state(target, moving_state);
        t.velocity = v.velocity;
        t.remainder = v.remainder;
        t.pair_id = v.pair_id;
        t.accel_mag = v.accel_mag;
        t.spin = v.spin;
        t.color = v.color;
        t.particle_id = v.particle_id;

        // Portable self-field: particle carries flux with it (up to K_B)
        double old_rho = v.density();
        if (old_rho > EPSILON_MAG) {
          double transfer = std::min(old_rho, K_B);
          double frac = transfer / old_rho;
          Vec3 self_field = v.flux * frac;
          v.flux = v.flux - self_field;
          t.flux = t.flux + self_field;

          // Dual-substrate: carry proportional L/R flux too
          if (rb.toggles.dual_substrate) {
            Vec3 sf_L = v.flux_L * frac;
            Vec3 sf_R = v.flux_R * frac;
            v.flux_L = v.flux_L - sf_L;
            v.flux_R = v.flux_R - sf_R;
            t.flux_L = t.flux_L + sf_L;
            t.flux_R = t.flux_R + sf_R;
          }
        }

        rb.set_state(i, 0);
        v.velocity = {};
        v.remainder = {};
        v.pair_id = -1;
        v.particle_id = -1;
        v.spin = 0;
        v.color = 0;
        rb.moved_[target] = 1;  // Prevent re-processing this tick
      } else if (t.state == v.state) {
        // Same sign: elastic bounce
        if (dx != 0) v.velocity.x *= -1.0;
        if (dy != 0) v.velocity.y *= -1.0;
        if (dz != 0) v.velocity.z *= -1.0;
        v.remainder = {};
      } else {
        // Opposite sign: annihilation — both particles return to void.
        Vec3 flux_v = v.flux;
        Vec3 flux_t = t.flux;
        Vec3 flux_v_L, flux_v_R, flux_t_L, flux_t_R;
        if (rb.toggles.dual_substrate) {
          flux_v_L = v.flux_L; flux_v_R = v.flux_R;
          flux_t_L = t.flux_L; flux_t_R = t.flux_R;
        }
        rb.set_state(i, 0);
        rb.set_state(target, 0);
        v.velocity = {}; t.velocity = {};
        v.remainder = {}; t.remainder = {};
        v.pair_id = -1; t.pair_id = -1;
        v.particle_id = -1; t.particle_id = -1;
        v.accel_mag = 0.0; t.accel_mag = 0.0;
        v.spin = 0; v.color = 0;
        t.spin = 0; t.color = 0;
        v.flux = {}; t.flux = {};
        if (rb.toggles.dual_substrate) {
          v.flux_L = {}; v.flux_R = {};
          t.flux_L = {}; t.flux_R = {};
        }
        // Distribute each particle's flux to its own neighbors
        auto nbrs_v = rb.lattice_.neighbors_6(i);
        auto nbrs_t = rb.lattice_.neighbors_6(target);
        for (int n : nbrs_v) rb.voxels_[n].flux += flux_v * (1.0 / 6.0);
        for (int n : nbrs_t) rb.voxels_[n].flux += flux_t * (1.0 / 6.0);
        if (rb.toggles.dual_substrate) {
          for (int n : nbrs_v) {
            rb.voxels_[n].flux_L += flux_v_L * (1.0 / 6.0);
            rb.voxels_[n].flux_R += flux_v_R * (1.0 / 6.0);
          }
          for (int n : nbrs_t) {
            rb.voxels_[n].flux_L += flux_t_L * (1.0 / 6.0);
            rb.voxels_[n].flux_R += flux_t_R * (1.0 / 6.0);
          }
        }
      }
    }
  } else {
    for (int i = 0; i < N; ++i) {
      auto &v = rb.voxels_[i];
      if (v.state == 0 || v.locked || rb.moved_[i]) continue;

      v.remainder += v.velocity * rb.dt_;

      auto c = rb.lattice_.coord(i);
      int dx = 0, dy = 0, dz = 0;

      if (v.remainder.x >= 1.0) { dx = 1; v.remainder.x -= 1.0; }
      else if (v.remainder.x <= -1.0) { dx = -1; v.remainder.x += 1.0; }
      if (v.remainder.y >= 1.0) { dy = 1; v.remainder.y -= 1.0; }
      else if (v.remainder.y <= -1.0) { dy = -1; v.remainder.y += 1.0; }
      if (v.remainder.z >= 1.0) { dz = 1; v.remainder.z -= 1.0; }
      else if (v.remainder.z <= -1.0) { dz = -1; v.remainder.z += 1.0; }

      if (dx == 0 && dy == 0 && dz == 0) continue;

      int target = rb.lattice_.index(c.x + dx, c.y + dy, c.z + dz);
      auto &t = rb.voxels_[target];

      if (t.state == 0) {
        // Move: transfer particle to target
        const int8_t moving_state = v.state;
        rb.set_state(target, moving_state);
        t.velocity = v.velocity;
        t.remainder = v.remainder;
        t.pair_id = v.pair_id;
        t.accel_mag = v.accel_mag;
        t.spin = v.spin;
        t.color = v.color;
        t.particle_id = v.particle_id;

        // Portable self-field: particle carries flux with it (up to K_B)
        double old_rho = v.density();
        if (old_rho > EPSILON_MAG) {
          double transfer = std::min(old_rho, K_B);
          double frac = transfer / old_rho;
          Vec3 self_field = v.flux * frac;
          v.flux = v.flux - self_field;
          t.flux = t.flux + self_field;

          // Dual-substrate: carry proportional L/R flux too
          if (rb.toggles.dual_substrate) {
            Vec3 sf_L = v.flux_L * frac;
            Vec3 sf_R = v.flux_R * frac;
            v.flux_L = v.flux_L - sf_L;
            v.flux_R = v.flux_R - sf_R;
            t.flux_L = t.flux_L + sf_L;
            t.flux_R = t.flux_R + sf_R;
          }
        }

        rb.set_state(i, 0);
        v.velocity = {};
        v.remainder = {};
        v.pair_id = -1;
        v.particle_id = -1;
        v.spin = 0;
        v.color = 0;
        rb.moved_[target] = 1;  // Prevent re-processing this tick
      } else if (t.state == v.state) {
        // Same sign: elastic bounce
        if (dx != 0) v.velocity.x *= -1.0;
        if (dy != 0) v.velocity.y *= -1.0;
        if (dz != 0) v.velocity.z *= -1.0;
        v.remainder = {};
      } else {
        // Opposite sign: annihilation — both particles return to void.
        Vec3 flux_v = v.flux;
        Vec3 flux_t = t.flux;
        Vec3 flux_v_L, flux_v_R, flux_t_L, flux_t_R;
        if (rb.toggles.dual_substrate) {
          flux_v_L = v.flux_L; flux_v_R = v.flux_R;
          flux_t_L = t.flux_L; flux_t_R = t.flux_R;
        }
        rb.set_state(i, 0);
        rb.set_state(target, 0);
        v.velocity = {}; t.velocity = {};
        v.remainder = {}; t.remainder = {};
        v.pair_id = -1; t.pair_id = -1;
        v.particle_id = -1; t.particle_id = -1;
        v.accel_mag = 0.0; t.accel_mag = 0.0;
        v.spin = 0; v.color = 0;
        t.spin = 0; t.color = 0;
        v.flux = {}; t.flux = {};
        if (rb.toggles.dual_substrate) {
          v.flux_L = {}; v.flux_R = {};
          t.flux_L = {}; t.flux_R = {};
        }
        // Distribute each particle's flux to its own neighbors
        auto nbrs_v = rb.lattice_.neighbors_6(i);
        auto nbrs_t = rb.lattice_.neighbors_6(target);
        for (int n : nbrs_v) rb.voxels_[n].flux += flux_v * (1.0 / 6.0);
        for (int n : nbrs_t) rb.voxels_[n].flux += flux_t * (1.0 / 6.0);
        if (rb.toggles.dual_substrate) {
          for (int n : nbrs_v) {
            rb.voxels_[n].flux_L += flux_v_L * (1.0 / 6.0);
            rb.voxels_[n].flux_R += flux_v_R * (1.0 / 6.0);
          }
          for (int n : nbrs_t) {
            rb.voxels_[n].flux_L += flux_t_L * (1.0 / 6.0);
            rb.voxels_[n].flux_R += flux_t_R * (1.0 / 6.0);
          }
        }
      }
    }
  }
}

}  // namespace ftd
