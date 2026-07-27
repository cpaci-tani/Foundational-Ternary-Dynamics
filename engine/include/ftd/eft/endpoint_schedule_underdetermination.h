#pragma once
/**
 * @file endpoint_schedule_underdetermination.h
 * @brief Endpoint insufficiency witness for spacetime current (FTD-0549).
 */

#include "ftd/voxel.h"

namespace ftd::eft {

struct EndpointScheduleUnderdeterminationResult {
  bool valid = false;
  int charge = 0;
  double displacement = 0.0;
  double epsilon = 0.0;
  Vec3 direction{};
  double monotonicity_margin = 0.0;
  double endpoint_position_residual = 0.0;
  double endpoint_derivative_residual = 0.0;
  double midpoint_derivative_residual = 0.0;
  Vec3 temporal_first_moment_difference{};
  Vec3 start_current_difference{};
  Vec3 end_current_difference{};
  Vec3 total_current_difference{};
  double analytic_moment_residual = 0.0;
  double split_recombination_residual = 0.0;
  double reversal_residual = 0.0;
  double schedule_split_norm = 0.0;
};

EndpointScheduleUnderdeterminationResult
evaluate_endpoint_schedule_underdetermination(
    double displacement,
    double epsilon,
    const Vec3& direction,
    int charge);

}  // namespace ftd::eft
