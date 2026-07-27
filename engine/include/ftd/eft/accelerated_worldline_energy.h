#pragma once
/**
 * @file accelerated_worldline_energy.h
 * @brief Exact uniform-force relativistic worldline observer (FTD-0547).
 */

#include "ftd/voxel.h"

namespace ftd::eft {

struct AcceleratedWorldlineEnergyResult {
  bool valid = false;
  double rest_energy = 0.0;
  double c_speed = 0.0;
  double temporal_scale = 0.0;
  double midpoint_momentum = 0.0;
  double half_impulse = 0.0;
  Vec3 direction{};
  double momentum_before = 0.0;
  double momentum_after = 0.0;
  double energy_before = 0.0;
  double energy_midpoint = 0.0;
  double energy_after = 0.0;
  double midpoint_velocity = 0.0;
  double secant_velocity = 0.0;
  double midpoint_displacement = 0.0;
  double exact_displacement = 0.0;
  double energy_change = 0.0;
  double midpoint_work = 0.0;
  double exact_work = 0.0;
  double midpoint_work_defect = 0.0;
  double exact_work_defect = 0.0;
  double defect_identity_residual = 0.0;
  double endpoint_residual = 0.0;
  double trajectory_derivative_residual = 0.0;
  double midpoint_schedule_deviation = 0.0;
  double causal_speed_excess = 0.0;
  double reversal_velocity_residual = 0.0;
  double reversal_trajectory_residual = 0.0;
  double leading_cubic_term = 0.0;
};

AcceleratedWorldlineEnergyResult evaluate_accelerated_worldline_energy(
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double midpoint_momentum,
    double half_impulse,
    const Vec3& direction);

}  // namespace ftd::eft
