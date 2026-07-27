#pragma once
/**
 * @file two_slab_variational_force.h
 * @brief Two-slab path variation of the selected Whitney action (FTD-0485).
 *
 * This observer differentiates the connection line action at a shared
 * worldline point.  It uses neither an E/B gather nor scalar-work division.
 * The initial scope is a reaction-free straight segment wholly inside one
 * spatial cell on each adjacent time slab.
 */

#include "ftd/eft/spacetime_worldline_coupling.h"

namespace ftd::eft {

struct TwoSlabVariationalForceResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  double coupling = 0.0;
  double temporal_scale = 0.0;
  Vec3 previous_position{};
  Vec3 shared_position{};
  Vec3 next_position{};
  Vec3 interaction_impulse{};
  double interaction_action = 0.0;
  double previous_deposit_action_residual = 0.0;
  double next_deposit_action_residual = 0.0;
  double connection_join_residual = 0.0;
};

/**
 * Evaluate D_2 S(previous,shared)+D_1 S(shared,next).
 *
 * Two-point Gauss-Legendre integration is algebraically exact here because
 * the within-cell Q1/Nedelec integrand is at most cubic in normalized time.
 * A three-component forward derivative supplies the full shared-point
 * gradient in the same evaluation.
 */
TwoSlabVariationalForceResult evaluate_two_slab_variational_force(
    const Vec3& previous_position,
    const Vec3& shared_position,
    const Vec3& next_position,
    int charge,
    const DualGaugePotentialSlab& previous_slab,
    const DualGaugePotentialSlab& next_slab,
    double coupling = 1.0);

}  // namespace ftd::eft
