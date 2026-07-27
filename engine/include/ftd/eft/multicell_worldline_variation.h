#pragma once
/**
 * @file multicell_worldline_variation.h
 * @brief Complete deposited-action variation through internal cell knots
 *        (FTD-0533).
 */

#include "ftd/eft/two_slab_variational_force.h"

#include <array>

namespace ftd::eft {

struct MulticellWorldlineVariationResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  int previous_internal_breaks = 0;
  int next_internal_breaks = 0;
  int maximum_simultaneous_crossing_multiplicity = 0;
  double coupling = 0.0;
  double temporal_scale = 0.0;
  double largest_step = 0.0;
  double interaction_action = 0.0;
  Vec3 interaction_impulse{};
  std::array<Vec3, 4> centered_gradient{};
  std::array<double, 4> maximum_one_sided_gap{};
  double final_centered_convergence_residual = 0.0;
  double minimum_resolved_one_sided_gap_ratio = 0.0;
  double maximum_directional_linearity_residual = 0.0;
  double connection_join_residual = 0.0;
  Vec3 previous_position{};
  Vec3 shared_position{};
  Vec3 next_position{};
};

/**
 * Differentiate the sum of two exact FTD-0484 deposited actions with respect
 * to their shared point.  Every displaced probe rebuilds the complete
 * cell-partitioned current, so internal crossing times move with the path.
 */
MulticellWorldlineVariationResult
evaluate_multicell_worldline_variation(
    const Vec3& previous_position,
    const Vec3& shared_position,
    const Vec3& next_position,
    int charge,
    const DualGaugePotentialSlab& previous_slab,
    const DualGaugePotentialSlab& next_slab,
    double coupling = 1.0,
    double largest_step = 0.0009765625);

}  // namespace ftd::eft

