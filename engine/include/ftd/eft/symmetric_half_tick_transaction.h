#pragma once
/**
 * @file symmetric_half_tick_transaction.h
 * @brief Observer-only source-centered half-kick / drift / half-kick transaction
 *        for the written interaction L_int = +G_C sum_x s divJ.
 *
 * FTD-0469.  This helper does not modify production dynamics.  It composes
 * the FTD-0468 common-action kick pair with the exact source-free
 * symplectic-Euler drift as one source-centered transaction:
 *
 *   half kick : W += (1/2) g,  p += (1/2) G_C s grad(div J),  g = -G_C grad s
 *   drift     : W += C_WAVE^2 lap J ; J += W
 *   half kick : W += (1/2) g,  p += (1/2) G_C s grad(div J')  (post-drift J)
 *
 * Exact ledger (all discrete, periodic, no continuum limit):
 *
 *   E_tick  = 0.5 W^T W + 0.5 J^T A J - 0.5 W^T A J        (A = -C_WAVE^2 L)
 *   H_int   = -G_C sum_x s divJ = -g^T J
 *   CT_sym  = +(1/4) g^T A J = +(1/4) G_C sum_x grad(s) . (C_WAVE^2 lap J)
 *
 * THEOREM (shadow energy): the source-centered transaction exactly conserves
 *   E_shadow = E_tick + H_int + CT_sym.
 * Proof: with A J* = g, the transaction is the exact source-free drift in
 * the deviation variables (J - J*, W - g/2); expanding E_tick in those
 * variables yields E_shadow up to an additive constant.  The coefficient
 * 1/4 is forced by the algebra, not fitted.
 *
 * THEOREM (naive total difference): the uncorrected ledger obeys
 *   [E_tick + H_int](t) - [E_tick + H_int](0) = CT_sym(0) - CT_sym(t),
 * a bounded total difference with no secular term.
 *
 * THEOREM (production-ordering invariant): the simultaneous full-kick
 * ordering (W += C_WAVE^2 lap J + g ; J += W, the coupled production form)
 * exactly conserves  E_tick + H_int + (1/2) g^T W  by the same deviation
 * argument with variables (J - J*, W).
 *
 * The map has an exact algebraic inverse, but it is NOT a self-adjoint
 * time integrator: the symplectic-Euler drift in the middle is not
 * self-adjoint, and centering the source kicks around it does not change
 * that fact.  "Source-centered" must not be promoted to "time-symmetric."
 *
 * Momentum: each half kick satisfies the FTD-0468 identity
 * Delta P_field + I_matter = 0 and the drift conserves the central field
 * momentum exactly (L and D_i commute; L D_i is skew-adjoint), so
 * p_matter + P_field is exactly conserved through whole transactions.
 */

#include "ftd/eft/fixed_j_recoil_capacity.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {

struct MatterMomentumLedger {
  std::vector<Vec3> by_site;

  explicit MatterMomentumLedger(std::size_t site_count = 0)
      : by_site(site_count) {}

  void ensure_size(std::size_t site_count) {
    if (by_site.size() != site_count) by_site.assign(site_count, {});
  }

  Vec3 total() const {
    Vec3 result{};
    for (const auto& momentum : by_site) result += momentum;
    return result;
  }

  double max_site_magnitude() const {
    double result = 0.0;
    for (const auto& momentum : by_site)
      result = std::max(result, momentum.mag());
    return result;
  }
};

/// Exact source-free symplectic-Euler drift (buffered, no source terms).
inline void advance_source_free_drift(RenderBridge& bridge) {
  const std::size_t count = bridge.voxels().size();
  std::vector<Vec3> next_flux(count);
  std::vector<Vec3> next_wave_vel(count);
  const double c2 = C_WAVE * C_WAVE;
  for (int index = 0; index < static_cast<int>(count); ++index) {
    next_wave_vel[static_cast<std::size_t>(index)] =
        bridge.voxels()[static_cast<std::size_t>(index)].wave_vel
        + bridge.laplacian_flux(index) * c2;
    next_flux[static_cast<std::size_t>(index)] =
        bridge.voxels()[static_cast<std::size_t>(index)].flux
        + next_wave_vel[static_cast<std::size_t>(index)];
  }
  for (std::size_t index = 0; index < count; ++index) {
    bridge.voxels()[index].flux = next_flux[index];
    bridge.voxels()[index].wave_vel = next_wave_vel[index];
  }
}

inline void reverse_source_free_drift(RenderBridge& bridge) {
  const std::size_t count = bridge.voxels().size();
  const double c2 = C_WAVE * C_WAVE;
  for (std::size_t index = 0; index < count; ++index)
    bridge.voxels()[index].flux -= bridge.voxels()[index].wave_vel;
  std::vector<Vec3> delta(count);
  for (int index = 0; index < static_cast<int>(count); ++index)
    delta[static_cast<std::size_t>(index)] =
        bridge.laplacian_flux(index) * c2;
  for (std::size_t index = 0; index < count; ++index)
    bridge.voxels()[index].wave_vel -= delta[index];
}

/// One interaction kick with weight w (w = 0.5 for the symmetric
/// transaction, w = 1.0 for a full kick).  Returns the matter impulse
/// deposited on the analysis momentum ledger.
inline Vec3 apply_interaction_kick(
    RenderBridge& bridge, double weight,
    MatterMomentumLedger* matter_ledger = nullptr) {
  Vec3 impulse{};
  const int count = static_cast<int>(bridge.voxels().size());
  if (matter_ledger != nullptr)
    matter_ledger->ensure_size(static_cast<std::size_t>(count));
  std::vector<Vec3> kick(static_cast<std::size_t>(count));
  for (int index = 0; index < count; ++index) {
    kick[static_cast<std::size_t>(index)] = gradient_state_op(
        bridge.voxels(), bridge.lattice(), index) * (-G_C * weight);
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    if (voxel.state != 0) {
      const Vec3 local_impulse = bridge.gradient_divergence(index)
          * (G_C * weight * static_cast<double>(voxel.state));
      impulse += local_impulse;
      if (matter_ledger != nullptr)
        matter_ledger->by_site[static_cast<std::size_t>(index)] +=
            local_impulse;
    }
  }
  for (int index = 0; index < count; ++index)
    bridge.voxels()[static_cast<std::size_t>(index)].wave_vel +=
        kick[static_cast<std::size_t>(index)];
  return impulse;
}

inline Vec3 remove_interaction_kick(
    RenderBridge& bridge, double weight,
    MatterMomentumLedger* matter_ledger = nullptr) {
  Vec3 impulse{};
  const int count = static_cast<int>(bridge.voxels().size());
  if (matter_ledger != nullptr)
    matter_ledger->ensure_size(static_cast<std::size_t>(count));
  std::vector<Vec3> kick(static_cast<std::size_t>(count));
  for (int index = 0; index < count; ++index) {
    kick[static_cast<std::size_t>(index)] = gradient_state_op(
        bridge.voxels(), bridge.lattice(), index) * (-G_C * weight);
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    if (voxel.state != 0) {
      const Vec3 local_impulse = bridge.gradient_divergence(index)
          * (G_C * weight * static_cast<double>(voxel.state));
      impulse += local_impulse;
      if (matter_ledger != nullptr)
        matter_ledger->by_site[static_cast<std::size_t>(index)] -=
            local_impulse;
    }
  }
  for (int index = 0; index < count; ++index)
    bridge.voxels()[static_cast<std::size_t>(index)].wave_vel -=
        kick[static_cast<std::size_t>(index)];
  return impulse;
}

/// Symmetric transaction forward step.  Accumulates the matter impulse into
/// `momentum` (analysis ledger only; production velocities are untouched).
inline void advance_symmetric_half_tick(RenderBridge& bridge,
                                        Vec3& momentum) {
  momentum += apply_interaction_kick(bridge, 0.5);
  advance_source_free_drift(bridge);
  momentum += apply_interaction_kick(bridge, 0.5);
}

/// Exact inverse of advance_symmetric_half_tick.
inline void reverse_symmetric_half_tick(RenderBridge& bridge,
                                        Vec3& momentum) {
  momentum -= remove_interaction_kick(bridge, 0.5);
  reverse_source_free_drift(bridge);
  momentum -= remove_interaction_kick(bridge, 0.5);
}

/// Per-manifested-site form used when kinetic energy must be evaluated.
/// Summing the site momenta reproduces the global FTD-0468 matter impulse,
/// while retaining the information required for a multiparticle energy.
inline void advance_symmetric_half_tick(
    RenderBridge& bridge, MatterMomentumLedger& matter_ledger) {
  apply_interaction_kick(bridge, 0.5, &matter_ledger);
  advance_source_free_drift(bridge);
  apply_interaction_kick(bridge, 0.5, &matter_ledger);
}

inline void reverse_symmetric_half_tick(
    RenderBridge& bridge, MatterMomentumLedger& matter_ledger) {
  remove_interaction_kick(bridge, 0.5, &matter_ledger);
  reverse_source_free_drift(bridge);
  remove_interaction_kick(bridge, 0.5, &matter_ledger);
}

/// Production-ordering control (full kick fused with drift), locked-particle
/// specialization of the coupled tick (zero velocity-coupling sector).
inline void advance_production_ordering(RenderBridge& bridge,
                                        Vec3& momentum) {
  momentum += apply_interaction_kick(bridge, 1.0);
  advance_source_free_drift(bridge);
}

/// Derived symmetric counterterm CT_sym = +(1/4) g^T A J
/// = +(1/4) G_C sum_x grad(s)(x) . (C_WAVE^2 lap J)(x).
inline long double symmetric_counterterm(const RenderBridge& bridge) {
  long double result = 0.0L;
  const double c2 = C_WAVE * C_WAVE;
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto gradient_state = gradient_state_op(
        bridge.voxels(), bridge.lattice(), index);
    result += 0.25L * static_cast<long double>(G_C)
        * dot_long_double(gradient_state,
                          bridge.laplacian_flux(index) * c2);
  }
  return result;
}

/// Production-ordering counterterm CT_prod = +(1/2) g^T W
/// = -(1/2) G_C sum_x grad(s)(x) . W(x).
inline long double production_counterterm(const RenderBridge& bridge) {
  long double result = 0.0L;
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto gradient_state = gradient_state_op(
        bridge.voxels(), bridge.lattice(), index);
    result -= 0.5L * static_cast<long double>(G_C)
        * dot_long_double(gradient_state,
                          bridge.voxels()[static_cast<std::size_t>(index)]
                              .wave_vel);
  }
  return result;
}

struct ShadowLedger {
  long double tick_energy = 0.0L;
  long double interaction = 0.0L;
  long double counterterm = 0.0L;
  long double shadow = 0.0L;
  long double naive = 0.0L;
  bool finite = true;
};

inline ShadowLedger measure_shadow_ledger(const RenderBridge& bridge) {
  ShadowLedger ledger;
  ledger.tick_energy = measure_native_wave_energy(bridge).tick_invariant;
  ledger.interaction = coupling_hamiltonian(bridge);
  ledger.counterterm = symmetric_counterterm(bridge);
  ledger.naive = ledger.tick_energy + ledger.interaction;
  ledger.shadow = ledger.naive + ledger.counterterm;
  ledger.finite = std::isfinite(static_cast<double>(ledger.shadow));
  return ledger;
}

}  // namespace ftd::eft
