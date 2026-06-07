/**
 * @file engine/src/render_bridge_phases/phase_forces.cpp
 * @purpose Implementation of phase_forces decomposition (Phase 4b, 2026-04-27).
 *
 * Extracted from render_bridge.cpp following the Phase 4a precedent
 * (phase_write.cpp) and the R1-R5 pattern (poisson_solvers.cpp,
 * transmutation_phases.cpp, energy_ledger_compute.cpp,
 * diagnostics_compute.cpp, injection.cpp). See ADR-0008.
 *
 * The original phase_forces() was ~225 LOC mixing:
 *   - prologue: optional Coulomb Poisson solve (warm-started SOR)
 *   - prologue: rebuild of colored_sites_cache_ for the color-force inner loop
 *   - main sequential per-voxel loop:
 *       * EM force (3 modes: emergent / poisson_coulomb / direct gradient)
 *       * Gravity force (tier-2 density gradient)
 *       * Lorentz force F = α·s·(v × B), B = curl(J)
 *       * Color force (3-regime SU(3)-flavoured profile w/ running α_s)
 *       * γ_FTD bandwidth-respecting relativistic momentum integration
 *
 * The extraction preserves the per-voxel loop body BYTE-IDENTICAL. The
 * golden tick test (test_render_bridge_golden) hashes 100 ticks to
 * 0xcd957b601d47868a and is the strict gate on this refactor: any drift
 * here is a physics bug.
 *
 * Why no per-force split: each force contributes to a single f_total that
 * is consumed by the relativistic-momentum integration in the same loop
 * iteration. Splitting the loop into per-force passes would require a
 * per-voxel f_total scratch buffer — a structural change that the golden
 * gate rejects. The Phase 4a precedent (single phase_write_main_loop)
 * applies here for the same reason: extract orchestration steps, keep
 * the loop body intact.
 */

#include "ftd/render_bridge_phases.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/field_operators.h"
#include <cmath>

namespace ftd {

void phase_forces_solve_potentials(RenderBridge& rb) {
  // Solve Coulomb potential (warm-started SOR)
  // Skip when emergent_forces is ON — force comes from flux field directly
  if (rb.toggles.poisson_coulomb && !rb.toggles.emergent_forces)
    rb.solve_coulomb_poisson();
}

void phase_forces_build_color_cache(RenderBridge& rb) {
  // PERF: colored_sites_cache_ is a bridge member — clear+push reuses capacity,
  // no per-tick malloc.
  rb.colored_sites_cache_.clear();
  if (rb.toggles.color_forces) {
    for (int ii : rb.ordered_active_indices()) {
      if (rb.voxels_[ii].state != 0 && rb.voxels_[ii].color != 0) {
        auto cc = rb.lattice_.coord(ii);
        rb.colored_sites_cache_.push_back({cc.x, cc.y, cc.z,
                                           rb.voxels_[ii].state, rb.voxels_[ii].color});
      }
    }
  }
}

void phase_forces_main_loop(RenderBridge& rb) {
  const int L = rb.lattice_.size();
  const auto& active = rb.ordered_active_indices();

#pragma omp parallel for schedule(static)
  for (int ai = 0; ai < static_cast<int>(active.size()); ++ai) {
    const int i = active[ai];
    auto &v = rb.voxels_[i];
    if (v.state == 0) continue;

    // EM force: three modes
    //   1. Poisson-based:  F = -alpha·s·∇φ_C        (standard, most accurate)
    //   2. Legacy gradient: F = -alpha·s·∇(∇·J)      (direct, short-range)
    //   3. Emergent (EFT):  F = G_C·s·∇|J|_{tier2}   (force from flux field)
    //      In mode 3, alpha = G_C² emerges: one G_C from this probe coupling,
    //      one G_C already embedded in the flux amplitude from the wave equation.
    Vec3 f_em;
    if (rb.toggles.emergent_forces) {
      // EFT emergent force: read force FROM the flux field established by
      // wave equation + Gauss constraint. No Poisson solver needed.
      // Use tier-2 stencil (r=2 neighbors) to avoid self-field contamination.
      auto ci = rb.lattice_.coord(i);
      int L = rb.lattice_.size();
      double grad_x = 0, grad_y = 0, grad_z = 0;
      // Tier-2 finite differences (skip r=1 to avoid self-field wake)
      auto safe = [&](int x, int y, int z) -> double {
        int wx = ((x % L) + L) % L;
        int wy = ((y % L) + L) % L;
        int wz = ((z % L) + L) % L;
        return rb.voxels_[rb.lattice_.index(wx, wy, wz)].density();
      };
      grad_x = (safe(ci.x+2, ci.y, ci.z) - safe(ci.x-2, ci.y, ci.z)) * 0.25;
      grad_y = (safe(ci.x, ci.y+2, ci.z) - safe(ci.x, ci.y-2, ci.z)) * 0.25;
      grad_z = (safe(ci.x, ci.y, ci.z+2) - safe(ci.x, ci.y, ci.z-2)) * 0.25;
      Vec3 grad_rho_t2 = {grad_x, grad_y, grad_z};
      // Force = G_C · state · ∇|J| (one vertex coupling; other G_C in flux)
      f_em = grad_rho_t2 * (G_C * v.state);
    } else if (rb.toggles.poisson_coulomb) {
      Vec3 grad_phi = rb.gradient_scalar(i, rb.phi_coulomb_);
      f_em = grad_phi * (-ALPHA * v.state);
    } else {
      Vec3 grad_divJ = rb.gradient_divergence(i);
      f_em = grad_divJ * (-ALPHA * v.state);
    }

    // Gravitational force from density gradient
    // Use tier-2 (r=2) stencil for manifested particles to avoid
    // self-field contamination at tier-1 (r=1) face-neighbors.
    // The self-field wake at r=1 creates an asymmetric density gradient
    // that causes spurious self-acceleration. At r=2 the self-field
    // influence is negligible and only external gradients contribute.
    Vec3 f_grav;
    if (rb.toggles.gravity) {
      auto c = rb.lattice_.coord(i);
      double dx = rb.voxels_[rb.lattice_.index(c.x+2, c.y, c.z)].density()
                - rb.voxels_[rb.lattice_.index(c.x-2, c.y, c.z)].density();
      double dy = rb.voxels_[rb.lattice_.index(c.x, c.y+2, c.z)].density()
                - rb.voxels_[rb.lattice_.index(c.x, c.y-2, c.z)].density();
      double dz = rb.voxels_[rb.lattice_.index(c.x, c.y, c.z+2)].density()
                - rb.voxels_[rb.lattice_.index(c.x, c.y, c.z-2)].density();
      Vec3 grad_rho = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
      f_grav = grad_rho * G_N;
    }

    // Lorentz (magnetic) force: F = α·s·(v × B) where B = curl(J)
    // From the Lagrangian velocity-coupling term L_vc = -g_c·s·(v·J),
    // the E-L equation yields F = g_c·q·(v × curl(J)).
    // With coupling g_c² = α, this gives F = α·s·(v × B).
    Vec3 f_lorentz;
    if (rb.toggles.lorentz_force && v.speed() > EPSILON_MAG) {
      Vec3 B = rb.curl_flux(i);
      f_lorentz = Vec3::cross(v.velocity, B) * (ALPHA * v.state);
    }

    // ── Color force: pairwise SU(3)-inspired interaction ─────────────
    // F_color(i←j) = α_s(r) · color_factor(c_i, c_j) · r̂ / r²
    // Running coupling α_s(r) implements asymptotic freedom at short r
    // and confinement saturation at large r.
    Vec3 f_color;
    if (rb.toggles.color_forces && v.color != 0) {
      auto ci = rb.lattice_.coord(i);
      for (auto& cs : rb.colored_sites_cache_) {
        // Skip self via coord equality (cheaper than carrying idx)
        if (cs.cx == ci.x && cs.cy == ci.y && cs.cz == ci.z) continue;
        double ddx = cs.cx - ci.x;
        double ddy = cs.cy - ci.y;
        double ddz = cs.cz - ci.z;
        if (ddx >  L/2) ddx -= L;
        if (ddx < -L/2) ddx += L;
        if (ddy >  L/2) ddy -= L;
        if (ddy < -L/2) ddy += L;
        if (ddz >  L/2) ddz -= L;
        if (ddz < -L/2) ddz += L;
        double r2 = ddx*ddx + ddy*ddy + ddz*ddz;
        double r = std::sqrt(r2);
        if (r < 1.0) r = 1.0;  // Clamp to lattice spacing (matches GPU)
        r2 = r * r;
        double cf = (v.color == cs.color) ? 0.5 : -1.0;
        double as = alpha_s_lattice(r);

        // Three-regime force profile (matches GPU kernels_forces.cu):
        //   r < COLOR_COULOMB_RADIUS:    Coulomb (asymptotic freedom)
        //   transition:                  Flux tube stretching
        //   r >= COLOR_TRANSITION_RADIUS: Linear confinement (constant string tension)
        double F_mag;
        if (r < COLOR_COULOMB_RADIUS) {
          F_mag = as * cf / r2;
        } else if (r < COLOR_TRANSITION_RADIUS) {
          F_mag = as * cf / (COLOR_TRANSITION_DENOM * r);
        } else {
          F_mag = as * cf * r / COLOR_LINEAR_DENOM;
        }

        // ddx points from probe to source; negate for repulsive force direction
        // Same color (cf>0): force pushes AWAY from source (repulsive)
        // Diff color (cf<0): force pulls TOWARD source (attractive)
        f_color.x -= F_mag * ddx / r;
        f_color.y -= F_mag * ddy / r;
        f_color.z -= F_mag * ddz / r;
      }
    }

    // BH-F3 (2026-05-05): canonical accel_mag is the RAW force magnitude from
    // EM + gravity + Lorentz only — Larmor radiation (the only consumer) is an
    // electromagnetic phenomenon, so colour shouldn't contribute, and we want
    // raw force not post-clamp realised |dv|/dt (which underestimates at the
    // bandwidth edge). GPU phase_forces_kernel writes the same quantity on the
    // same code path, so accel_mag is bit-exact CPU↔GPU at unit mass.
    Vec3 f_em_grav_lorentz = f_em + f_grav + f_lorentz;
    Vec3 f_total = f_em_grav_lorentz + f_color;

    // Store for diagnostics
    rb.force_diag_[i].f_coulomb = f_em;
    rb.force_diag_[i].f_gravity = f_grav;
    rb.force_diag_[i].f_strong = f_color;
    rb.force_diag_[i].f_magnetic = f_lorentz;
    rb.force_diag_[i].f_exchange = {};

    // Record acceleration magnitude (EM + grav + Lorentz; colour excluded; see BH-F3)
    v.accel_mag = f_em_grav_lorentz.mag();

    // Apply force (skip locked particles)
    if (!v.locked) {
      // ── γ_FTD MOMENTUM INTEGRATION (2026-04-17, TRACKER §1.2) ──────
      //
      // FTD bandwidth postulate: v²/C² + L² < 1, where C = C_SPEED and
      // L is local topological latency (gravity).  The corresponding
      // Lorentz factor is γ_FTD = 1/√(1 − v²/C² − L²).
      //
      // To respect this constraint exactly, we integrate MOMENTUM, not
      // velocity: p = γ_FTD · v.  Newton's law becomes dp/dt = F, and
      // v is extracted from p at the end of the step.  This guarantees
      // |v| → C·√(1 − L²) asymptotically as force → ∞; no clamp, no
      // energy discard, Lorentz-invariant by construction.
      //
      // Algebra (derivation in TRACKER §1.2):
      //   γ²|v|² = |p|²
      //   γ² = 1/(1 − |v|²/C² − L²)
      //   ⇒  |v|² = C²(1 − L²) · |p|² / (C² + |p|²)
      //   ⇒  v⃗   = p⃗ · C · √((1 − L²) / (C² + |p|²))
      //
      // Newtonian limit (|v| << C, L = 0): γ → 1, p ≈ v, v_new ≈ v + F·dt. ✓
      // Ultra-relativistic (|p| → ∞):       |v| → C·√(1 − L²).          ✓
      // Horizon (L → 1):                    |v| → 0.                     ✓
      //
      // Superseded the previous non-relativistic clamp
      // `if (|v| > C) v *= C/|v|;` which discarded energy and was also
      // STRICTER than the true bandwidth (clamp allowed |v| ≤ C(1−L²);
      // FTD bandwidth allows |v| ≤ C·√(1−L²)).
      const double C      = C_SPEED;
      const double C2     = C * C;
      const double L      = v.latency;                  // 0 if latency_field off
      const double L2     = L * L;
      // Budget-safe: clamp 1−L² strictly positive so sqrt() never
      // underflows at or near the horizon. RF-8 (2026-04-25): use the
      // shared BANDWIDTH_FLOOR constant from constants.h instead of bare 1e-6.
      const double one_L2 = std::max(1.0 - L2, BANDWIDTH_FLOOR);

      // Current γ — BANDWIDTH_FLOOR keeps γ finite when the previous tick
      // left v at the bandwidth edge.
      const double v2 = v.velocity.mag2();
      double budget  = v2 / C2 + L2;
      if (budget > 1.0 - BANDWIDTH_FLOOR) budget = 1.0 - BANDWIDTH_FLOOR;
      const double gamma_in = 1.0 / std::sqrt(1.0 - budget);

      // Reconstruct momentum, apply force, extract new velocity.
      Vec3 p = v.velocity * gamma_in;
      p = p + f_total * rb.dt_;
      const double p2 = p.mag2();
      const double scale = C * std::sqrt(one_L2 / (C2 + p2));
      v.velocity = p * scale;
    }
  }
}

// ── Phase 2 (unified mass): rigid-body cluster inertia ────────────────────
// A connected cluster of N LOCKED manifested voxels (same state sign, 26-Moore
// connectivity) carries inertial mass N·M_REST. Its centre of mass integrates
// a_COM = F_cluster/(N·M_REST) using the SAME γ_FTD momentum scheme as the
// per-voxel loop, with the per-mass force F_cluster/(N·M_REST) in place of
// f_total; the resulting V_COM is written to every member (rigid body).
//
// The per-voxel loop already skips locked voxels (the `if (!v.locked)` guard
// above), so this pass is purely ADDITIVE: with cluster_inertia OFF it never
// runs and the golden hash is byte-identical. F_cluster is reconstructed
// EXACTLY from force_diag_ (= f_coulomb + f_gravity + f_strong + f_magnetic =
// f_em + f_grav + f_lorentz + f_color, written for every voxel above).
//
// Phase 2 is the INERTIAL (velocity) response only — locked members stay frozen
// in POSITION (phase_movement still skips them); turning V_COM into an actual
// lattice trajectory is Phase 3. The traversal is sequential + deterministic so
// the float-summation order is fixed (bit-exact; the GPU path runs this same
// host code on synced data → bit-exact CPU↔GPU by construction).
void phase_forces_integrate_clusters(RenderBridge& rb) {
  const auto& active = rb.ordered_active_indices();
  if (active.empty()) return;

  std::vector<char> visited(rb.voxels_.size(), 0);
  std::vector<int>  stack;
  std::vector<int>  members;
  const double C  = C_SPEED;
  const double C2 = C * C;

  for (int seed : active) {
    if (visited[seed]) continue;
    visited[seed] = 1;
    auto& sv = rb.voxels_[seed];
    if (sv.state == 0 || !sv.locked) continue;
    const int sign = (sv.state > 0) ? 1 : -1;

    // Flood-fill this locked, same-sign cluster (26-connectivity).
    members.clear();
    stack.clear();
    stack.push_back(seed);
    int    N = 0;
    Vec3   F_cluster{};
    Vec3   sum_vel{};
    double sum_lat = 0.0;
    while (!stack.empty()) {
      const int cur = stack.back();
      stack.pop_back();
      auto& cv = rb.voxels_[cur];
      members.push_back(cur);
      ++N;
      const auto& fd = rb.force_diag_[cur];
      F_cluster = F_cluster + fd.f_coulomb + fd.f_gravity + fd.f_strong + fd.f_magnetic;
      sum_vel   = sum_vel + cv.velocity;
      sum_lat  += cv.latency;
      for (int nb : rb.lattice_.neighbors_26(cur)) {
        if (visited[nb]) continue;
        auto& nv = rb.voxels_[nb];
        if (nv.state == 0 || !nv.locked) continue;
        if (((nv.state > 0) ? 1 : -1) != sign) continue;
        visited[nb] = 1;
        stack.push_back(nb);
      }
    }
    if (N == 0) continue;

    // γ_FTD integration of the COM at inertial mass m = N·M_REST.
    // Identical algebra to the per-voxel loop with v→V_COM, f_total→F_cluster/m.
    const double m        = static_cast<double>(N) * M_REST;
    Vec3         V_COM    = sum_vel * (1.0 / N);
    const double L        = sum_lat / N;            // mean member latency
    const double L2       = L * L;
    const double one_L2   = std::max(1.0 - L2, BANDWIDTH_FLOOR);
    double       budget   = V_COM.mag2() / C2 + L2;
    if (budget > 1.0 - BANDWIDTH_FLOOR) budget = 1.0 - BANDWIDTH_FLOOR;
    const double gamma_in = 1.0 / std::sqrt(1.0 - budget);
    Vec3         q        = V_COM * gamma_in;                 // P/m
    q = q + (F_cluster * (1.0 / m)) * rb.dt_;                 // a = F/m
    const double q2       = q.mag2();
    const double scale    = C * std::sqrt(one_L2 / (C2 + q2));
    V_COM = q * scale;

    for (int midx : members) rb.voxels_[midx].velocity = V_COM;
  }
}

}  // namespace ftd
