#pragma once
/**
 * @file fixed_step_energy_scope.h
 * @brief Exact fixed-step variational energy witness (FTD-0543).
 */

namespace ftd::eft {

struct FixedStepEnergyScopeResult {
  double q0 = 0.0;
  double q1 = 0.0;
  double step = 0.0;

  double midpoint_p0 = 0.0;
  double midpoint_p1 = 0.0;
  double midpoint_energy0 = 0.0;
  double midpoint_energy1 = 0.0;
  double midpoint_energy_defect = 0.0;
  double analytic_energy_defect = 0.0;
  double midpoint_identity_residual = 0.0;
  double discrete_lagrangian_energy = 0.0;

  double discrete_gradient = 0.0;
  double gradient_p0 = 0.0;
  double gradient_p1 = 0.0;
  double gradient_energy0 = 0.0;
  double gradient_energy1 = 0.0;
  double gradient_energy_defect = 0.0;
  double gradient_area_determinant = 0.0;
  double gradient_area_defect = 0.0;
  bool valid = false;
};

FixedStepEnergyScopeResult evaluate_fixed_step_energy_scope(
    double q0, double q1, double step);

}  // namespace ftd::eft
