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
 * golden tick test (test_render_bridge_golden) hashes 100 ticks to the pinned
 * GOLDEN_HASH (current value lives in test_render_bridge_golden.cpp) and is
 * the strict gate on this refactor: any drift here is a physics bug.
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
#include "ftd/causal_kinematics.h"
#include "ftd/voxel_rng.h"
#include "ftd/movement_order.h"
#include <algorithm>
#include <vector>
#include <numeric>

namespace ftd {

namespace {

enum class BoundaryOutcome { Proceed, Handled };

bool project_for_movement(Voxel& v) {
  const double scale = movement_projection_scale(v.latency, v.velocity.mag2());
  if (scale >= 1.0) return false;
  if (scale > 0.0) {
    v.velocity *= scale;
  } else {
    // Literal assignment avoids inf*0 -> NaN for externally corrupted input.
    v.velocity = {};
  }
  return true;
}

// When reflective_boundary is OFF, a particle that would cross a face is
// removed (energy exhausts into the void — no toroidal wrap). When ON, it
// mirror-bounces at the face like an elastic wall collision.
BoundaryOutcome handle_face_crossing(RenderBridge& rb, Voxel& v, int dx, int dy, int dz, int i) {
  auto c = rb.lattice().coord(i);
  const int nx = c.x + dx;
  const int ny = c.y + dy;
  const int nz = c.z + dz;
  const int L = rb.lattice().size();

  const bool crosses = (nx < 0 || nx >= L || ny < 0 || ny >= L || nz < 0 || nz >= L);
  if (!crosses) return BoundaryOutcome::Proceed;

  if (rb.toggles.reflective_boundary) {
    if (dx != 0) v.velocity.x *= -1.0;
    if (dy != 0) v.velocity.y *= -1.0;
    if (dz != 0) v.velocity.z *= -1.0;
    v.remainder = {};
    return BoundaryOutcome::Handled;
  }

  rb.set_state(i, 0);
  v.velocity = {};
  v.remainder = {};
  v.pair_id = -1;
  v.particle_id = -1;
  v.spin = 0;
  v.color = 0;
  v.flux = {};
  if (rb.toggles.dual_substrate) {
    v.flux_L = {};
    v.flux_R = {};
  }
  return BoundaryOutcome::Handled;
}

}  // namespace

void phase_movement_main_loop(RenderBridge& rb) {
  const int N = static_cast<int>(rb.lattice_.total_sites());
  std::fill(rb.moved_.begin(), rb.moved_.end(), 0);

  if (rb.toggles.symmetric_movement_order) {
    if (static_cast<int>(rb.movement_indices_.size()) != N) {
      rb.movement_indices_.resize(N);
    }
    auto& indices = rb.movement_indices_;
    std::iota(indices.begin(), indices.end(), 0);
    const std::uint64_t seed =
        static_cast<std::uint64_t>(rb.toggles.langevin_seed);
    for (int n = N - 1; n > 0; --n) {
      const int j = movement_shuffle_j(seed, n, rb.tick_);
      std::swap(indices[n], indices[j]);
    }

    for (int i : indices) {
      auto &v = rb.voxels_[i];
      if (v.state == 0 || v.locked || rb.moved_[i]) continue;

      if (project_for_movement(v)) ++rb.causal_projection_events_this_tick_;

      v.remainder += v.velocity * rb.dt_;

      auto c = rb.lattice_.coord(i);
      int dx = 0, dy = 0, dz = 0;
      extract_remainder_hops(v.remainder.x, v.remainder.y, v.remainder.z,
                             dx, dy, dz, true, seed, i, rb.tick_);

      if (dx == 0 && dy == 0 && dz == 0) continue;

      if (handle_face_crossing(rb, v, dx, dy, dz, i) == BoundaryOutcome::Handled) continue;

      int target = rb.lattice_.index(c.x + dx, c.y + dy, c.z + dz);
      auto &t = rb.voxels_[target];

      if (t.state == 0) {
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        const auto history_before_i = eft::capture_history_site(i, v);
        const auto history_before_target = eft::capture_history_site(target, t);
        // FTD-HISTORY-END
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
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        if (rb.history_journal_enabled()) {
          eft::HistoryEvent event;
          event.kind = eft::HistoryEventKind::Movement;
          event.tick = rb.tick_;
          event.site_count = 2;
          event.before[0] = history_before_i;
          event.before[1] = history_before_target;
          event.after[0] = eft::capture_history_site(i, v);
          event.after[1] = eft::capture_history_site(target, t);
          rb.record_history_event(event);
        }
        // FTD-HISTORY-END
      } else if (t.state == v.state) {
        // Same sign: elastic bounce
        if (dx != 0) v.velocity.x *= -1.0;
        if (dy != 0) v.velocity.y *= -1.0;
        if (dz != 0) v.velocity.z *= -1.0;
        v.remainder = {};
      } else {
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        const auto history_before_i = eft::capture_history_site(i, v);
        const auto history_before_target = eft::capture_history_site(target, t);
        // FTD-HISTORY-END
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
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        if (rb.history_journal_enabled()) {
          eft::HistoryEvent event;
          event.kind = eft::HistoryEventKind::Annihilation;
          event.tick = rb.tick_;
          event.site_count = 2;
          event.before[0] = history_before_i;
          event.before[1] = history_before_target;
          event.after[0] = eft::capture_history_site(i, v);
          event.after[1] = eft::capture_history_site(target, t);
          rb.record_history_event(event);
        }
        // FTD-HISTORY-END
      }
    }
  } else {
    for (int i = 0; i < N; ++i) {
      auto &v = rb.voxels_[i];
      if (v.state == 0 || v.locked || rb.moved_[i]) continue;

      if (project_for_movement(v)) ++rb.causal_projection_events_this_tick_;

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

      if (handle_face_crossing(rb, v, dx, dy, dz, i) == BoundaryOutcome::Handled) continue;

      int target = rb.lattice_.index(c.x + dx, c.y + dy, c.z + dz);
      auto &t = rb.voxels_[target];

      if (t.state == 0) {
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        const auto history_before_i = eft::capture_history_site(i, v);
        const auto history_before_target = eft::capture_history_site(target, t);
        // FTD-HISTORY-END
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
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        if (rb.history_journal_enabled()) {
          eft::HistoryEvent event;
          event.kind = eft::HistoryEventKind::Movement;
          event.tick = rb.tick_;
          event.site_count = 2;
          event.before[0] = history_before_i;
          event.before[1] = history_before_target;
          event.after[0] = eft::capture_history_site(i, v);
          event.after[1] = eft::capture_history_site(target, t);
          rb.record_history_event(event);
        }
        // FTD-HISTORY-END
      } else if (t.state == v.state) {
        // Same sign: elastic bounce
        if (dx != 0) v.velocity.x *= -1.0;
        if (dy != 0) v.velocity.y *= -1.0;
        if (dz != 0) v.velocity.z *= -1.0;
        v.remainder = {};
      } else {
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        const auto history_before_i = eft::capture_history_site(i, v);
        const auto history_before_target = eft::capture_history_site(target, t);
        // FTD-HISTORY-END
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
        // FTD-HISTORY-BEGIN: observation-only native event journal.
        if (rb.history_journal_enabled()) {
          eft::HistoryEvent event;
          event.kind = eft::HistoryEventKind::Annihilation;
          event.tick = rb.tick_;
          event.site_count = 2;
          event.before[0] = history_before_i;
          event.before[1] = history_before_target;
          event.after[0] = eft::capture_history_site(i, v);
          event.after[1] = eft::capture_history_site(target, t);
          rb.record_history_event(event);
        }
        // FTD-HISTORY-END
      }
    }
  }
}

}  // namespace ftd
