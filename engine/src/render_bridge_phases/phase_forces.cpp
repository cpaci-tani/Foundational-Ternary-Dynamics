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
 * golden tick test (test_render_bridge_golden) hashes 100 ticks to the pinned
 * GOLDEN_HASH (current value lives in test_render_bridge_golden.cpp) and is
 * the strict gate on this refactor: any drift here is a physics bug.
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
#include "ftd/parallel.h"
#include "ftd/strong_stress_energy.h"
#include <cmath>

namespace ftd {

void phase_forces_solve_potentials(RenderBridge& rb) {
  // Coulomb Poisson is an EM channel. Yukawa/exchange/color-only ticks must
  // not solve φ_C (GPU gpu_phase_forces is similarly gated on toggles.forces).
  if (!rb.toggles.forces) return;
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
        const auto& v = rb.voxels_[ii];
        rb.colored_sites_cache_.push_back({
            ii, cc.x, cc.y, cc.z,
            static_cast<double>(cc.x) + v.remainder.x,
            static_cast<double>(cc.y) + v.remainder.y,
            static_cast<double>(cc.z) + v.remainder.z,
            v.state, v.color});
      }
    }
  }
}

void phase_forces_main_loop(RenderBridge& rb) {
  const int L = rb.lattice_.size();
  const auto& active = rb.ordered_active_indices();

  ftd::parallel_for(0, static_cast<int>(active.size()), [&](int _lo, int _hi) {
  for (int ai = _lo; ai < _hi; ++ai) {
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
    Vec3 f_grav;
    Vec3 f_lorentz;
    if (rb.toggles.forces) {
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
    if (rb.toggles.gravity) {
      auto c = rb.lattice_.coord(i);
      if (rb.toggles.geometric_gravity) {
        // FTD-1016: Q0 weak EOM F = M_INERTIAL C² ℒ ∇ℒ. Same tier-2
        // stencil as the density path. Default-off; golden-neutral.
        double dx = rb.voxels_[rb.lattice_.index(c.x+2, c.y, c.z)].latency
                  - rb.voxels_[rb.lattice_.index(c.x-2, c.y, c.z)].latency;
        double dy = rb.voxels_[rb.lattice_.index(c.x, c.y+2, c.z)].latency
                  - rb.voxels_[rb.lattice_.index(c.x, c.y-2, c.z)].latency;
        double dz = rb.voxels_[rb.lattice_.index(c.x, c.y, c.z+2)].latency
                  - rb.voxels_[rb.lattice_.index(c.x, c.y, c.z-2)].latency;
        Vec3 grad_L = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
        f_grav = grad_L * (M_INERTIAL * C_SPEED * C_SPEED * v.latency);
      } else {
        double dx = rb.voxels_[rb.lattice_.index(c.x+2, c.y, c.z)].density()
                  - rb.voxels_[rb.lattice_.index(c.x-2, c.y, c.z)].density();
        double dy = rb.voxels_[rb.lattice_.index(c.x, c.y+2, c.z)].density()
                  - rb.voxels_[rb.lattice_.index(c.x, c.y-2, c.z)].density();
        double dz = rb.voxels_[rb.lattice_.index(c.x, c.y, c.z+2)].density()
                  - rb.voxels_[rb.lattice_.index(c.x, c.y, c.z-2)].density();
        Vec3 grad_rho = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
        f_grav = grad_rho * G_N;
      }
    }

    // Selected Lorentz-shaped matter force: F = α·s·(v × B), B=curl(J).
    // The onsite point coupling -g_c*s*(v·J) supplies this operator shape,
    // while α=g_c² is the selected effective normalization used here.
    // FTD-0574 proves this is NOT the common-action partner of phase_read's
    // +g_c*curl(s*v) field source: that source requires +g_c<curl J,s*v> and
    // its reciprocal path variation contains induction and curl-curl terms.
    if (rb.toggles.lorentz_force && v.speed() > EPSILON_MAG) {
      Vec3 B = rb.curl_flux(i);
      f_lorentz = Vec3::cross(v.velocity, B) * (ALPHA * v.state);
    }
    } // rb.toggles.forces — EM / gravity / Lorentz only

    // ── Color force: pairwise SU(3)-inspired interaction ─────────────
    // F_color(i←j) = α_s(r) · color_factor(c_i, c_j) · r̂ / r²
    // Running coupling α_s(r) implements asymptotic freedom at short r
    // and confinement saturation at large r.
    Vec3 f_color;
    if (rb.toggles.color_forces && v.color != 0) {
      auto ci = rb.lattice_.coord(i);
      for (auto& cs : rb.colored_sites_cache_) {
        if (cs.idx == i) continue;
        const bool continuous = rb.toggles.strong_stress_energy;
        const double pix = static_cast<double>(ci.x) + (continuous ? v.remainder.x : 0.0);
        const double piy = static_cast<double>(ci.y) + (continuous ? v.remainder.y : 0.0);
        const double piz = static_cast<double>(ci.z) + (continuous ? v.remainder.z : 0.0);
        double ddx = (continuous ? cs.px : static_cast<double>(cs.cx)) - pix;
        double ddy = (continuous ? cs.py : static_cast<double>(cs.cy)) - piy;
        double ddz = (continuous ? cs.pz : static_cast<double>(cs.cz)) - piz;
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
        //   r >= COLOR_TRANSITION_RADIUS: harmonic F∝r, or linear SIGMA_STRING
        //                                 when toggles.confinement is on.
        double F_mag;
        if (continuous) {
          F_mag = cf * strong_radial_profile(r);
        } else {
          F_mag = color_regime_force_mag(r, as, cf, rb.toggles.confinement);
        }

        // ddx points from probe to source; negate for repulsive force direction
        // Same color (cf>0): force pushes AWAY from source (repulsive)
        // Diff color (cf<0): force pulls TOWARD source (attractive)
        f_color.x -= F_mag * ddx / r;
        f_color.y -= F_mag * ddy / r;
        f_color.z -= F_mag * ddz / r;
      }
    }

    // Yukawa (strong_force) and exchange: same formulas as kernels_forces.cu.
    // Independent of toggles.forces so a Yukawa-only tick does not run EM.
    Vec3 f_yukawa;
    if (rb.toggles.strong_force) {
      auto ci = rb.lattice_.coord(i);
      for (int aj : active) {
        const int j = aj;
        if (j == i) continue;
        const auto& w = rb.voxels_[j];
        if (w.state == 0) continue;
        auto cj = rb.lattice_.coord(j);
        const double ddx = static_cast<double>(lattice_periodic_delta(cj.x, ci.x, L));
        const double ddy = static_cast<double>(lattice_periodic_delta(cj.y, ci.y, L));
        const double ddz = static_cast<double>(lattice_periodic_delta(cj.z, ci.z, L));
        const double r2 = ddx * ddx + ddy * ddy + ddz * ddz;
        double r = std::sqrt(r2);
        const double F_mag = yukawa_pair_force_mag(r);
        if (r < 1.0) r = 1.0;
        // Attractive: toward j, matching yukawa_force_kernel (+= dx * f_mag / r).
        f_yukawa.x += F_mag * ddx / r;
        f_yukawa.y += F_mag * ddy / r;
        f_yukawa.z += F_mag * ddz / r;
      }
    }
    Vec3 f_exchange;
    if (rb.toggles.exchange_force && v.spin != 0) {
      auto ci = rb.lattice_.coord(i);
      for (int aj : active) {
        const int j = aj;
        if (j == i) continue;
        const auto& w = rb.voxels_[j];
        if (w.state == 0 || w.spin != v.spin) continue;
        auto cj = rb.lattice_.coord(j);
        const double ddx = static_cast<double>(lattice_periodic_delta(cj.x, ci.x, L));
        const double ddy = static_cast<double>(lattice_periodic_delta(cj.y, ci.y, L));
        const double ddz = static_cast<double>(lattice_periodic_delta(cj.z, ci.z, L));
        const double r2 = ddx * ddx + ddy * ddy + ddz * ddz;
        double r = std::sqrt(r2);
        const double F_mag = exchange_pair_force_mag(r, r2);
        if (r < 1.0) r = 1.0;
        // Repulsive: away from j, matching exchange_force_kernel.
        f_exchange.x -= F_mag * ddx / r;
        f_exchange.y -= F_mag * ddy / r;
        f_exchange.z -= F_mag * ddz / r;
      }
    }

    // BH-F3 (2026-05-05): canonical accel_mag is the RAW force magnitude from
    // EM + gravity + Lorentz only — Larmor radiation (the only consumer) is an
    // electromagnetic phenomenon, so colour shouldn't contribute, and we want
    // raw force not post-clamp realised |dv|/dt (which underestimates at the
    // bandwidth edge). GPU phase_forces_kernel writes the same quantity on the
    // same code path, so accel_mag is bit-exact CPU↔GPU at unit mass.
    Vec3 f_em_grav_lorentz = f_em + f_grav + f_lorentz;
    Vec3 f_strong = f_color + f_yukawa;
    Vec3 f_total = f_em_grav_lorentz + f_strong + f_exchange;

    // Store for diagnostics
    rb.force_diag_[i].f_coulomb = f_em;
    rb.force_diag_[i].f_gravity = f_grav;
    rb.force_diag_[i].f_strong = f_strong;
    rb.force_diag_[i].f_magnetic = f_lorentz;
    rb.force_diag_[i].f_exchange = f_exchange;

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
      // |v| → C·√(1 − L²) asymptotically as force → ∞; no clamp and no
      // energy discard. This preserves the selected causal budget; it is not
      // a theorem of Lorentz covariance.
      //
      // Algebra (derivation in TRACKER §1.2):
      //   γ²|v|² = |p|²
      //   γ² = 1/(1 − |v|²/C² − L²)
      //   ⇒  |v|² = C²(1 − L²) · |p|² / (C² + |p|²)
      //   ⇒  v⃗   = p⃗ · C · √((1 − L²) / (C² + |p|²))
      //
      // Newtonian limit: q=P/M≈u and dq/dt=F/M_INERTIAL.
      // Ultra-relativistic (|p| → ∞):       |v| → C·√(1 − L²).          ✓
      // Horizon (L → 1):                    |v| → 0.                     ✓
      //
      // Superseded the previous non-relativistic clamp
      // `if (|v| > C) v *= C/|v|;` which discarded energy and was also
      // STRICTER than the true bandwidth (clamp allowed |v| ≤ C(1−L²);
      // FTD bandwidth allows |v| ≤ C·√(1−L²)).
      const double L      = v.latency;                  // 0 if latency_field off
      const double v2 = v.velocity.mag2();
      const double gamma_in = momentum_input_gamma(L, v2);

      // Reconstruct specific momentum q=P/M, apply F/M, extract raw u.
      Vec3 q = v.velocity * gamma_in;
      q = q + f_total * (rb.dt_ / M_INERTIAL);
      const double scale = specific_momentum_velocity_scale(L, q.mag2());
      v.velocity = scale > 0.0 ? q * scale : Vec3{};
    }
  }
  });
}

// ── Phase 2 (unified mass): rigid-body cluster inertia ────────────────────
// A connected cluster of N LOCKED manifested voxels (same state sign, 26-Moore
// connectivity) carries inertial mass N·M_INERTIAL. Its centre of mass integrates
// a_COM = F_cluster/(N·M_INERTIAL) using the SAME γ_FTD momentum scheme as the
// per-voxel loop, with the per-mass force F_cluster/(N·M_INERTIAL) in place of
// f_total; the resulting V_COM is written to every member (rigid body).
//
// The per-voxel loop already skips locked voxels (the `if (!v.locked)` guard
// above), so this pass is purely ADDITIVE: with cluster_inertia OFF it never
// runs and the golden hash is byte-identical. F_cluster is reconstructed
// EXACTLY from force_diag_ (= f_coulomb + f_gravity + f_strong + f_magnetic
// + f_exchange, written for every voxel above).
//
// Phase 2 is the INERTIAL (velocity) response only — locked members stay frozen
// in POSITION (phase_movement still skips them); turning V_COM into an actual
// lattice trajectory is Phase 3. The traversal is sequential + deterministic so
// the float-summation order is fixed (bit-exact; the GPU path runs this same
// host code on synced data → bit-exact CPU↔GPU by construction).
void phase_forces_integrate_clusters(RenderBridge& rb) {
  const auto& active = rb.ordered_active_indices();
  if (active.empty()) return;

  const std::size_t N_voxels = rb.voxels_.size();
  if (rb.cluster_visited_.size() != N_voxels) {
    rb.cluster_visited_.resize(N_voxels, 0);
  }
  std::fill(rb.cluster_visited_.begin(), rb.cluster_visited_.end(), 0);
  auto& visited = rb.cluster_visited_;

  rb.cluster_stack_.clear();
  auto& stack = rb.cluster_stack_;

  rb.cluster_members_.clear();
  auto& members = rb.cluster_members_;

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
      F_cluster = F_cluster + fd.f_coulomb + fd.f_gravity + fd.f_strong
                + fd.f_magnetic + fd.f_exchange;
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

    // γ_FTD integration of the COM at inertial mass m = N·M_INERTIAL.
    // Identical algebra to the per-voxel loop with v→V_COM, f_total→F_cluster/m.
    const double m        = static_cast<double>(N) * M_INERTIAL;
    Vec3         V_COM    = sum_vel * (1.0 / N);
    const double L        = sum_lat / N;            // mean member latency
    const double gamma_in = momentum_input_gamma(L, V_COM.mag2());
    Vec3         q        = V_COM * gamma_in;                 // P/m
    q = q + (F_cluster * (1.0 / m)) * rb.dt_;                 // a = F/m
    const double q2       = q.mag2();
    const double scale    = specific_momentum_velocity_scale(L, q2);
    V_COM = scale > 0.0 ? q * scale : Vec3{};

    for (int midx : members) rb.voxels_[midx].velocity = V_COM;
  }
}

}  // namespace ftd
