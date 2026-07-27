#pragma once
/**
 * @file dressing_fiber_ledger.h
 * @brief Minimal history-fiber bookkeeping for cusp work (FTD-0495).
 *
 * Observer only.  The real fiber coordinate is not a production Voxel field.
 */

#include "ftd/eft/cusp_dressing_integrability.h"

namespace ftd::eft {

struct DressingFiberStepResult {
  bool valid = false;
  double dressing_before = 0.0;
  double dressing_after = 0.0;
  double dressing_change = 0.0;
  double field_energy_change = 0.0;
  double matter_energy_change = 0.0;
  double extended_energy_residual = 0.0;
  double reverse_dressing_residual = 0.0;

  double multiplier = 0.0;
  double constraint_derivative_D0 = 0.0;
  double constraint_derivative_D1 = 0.0;
  Vec3 cusp_position_gradient{};
  Vec3 constraint_position_gradient{};
  double action_gradient_residual = 0.0;
};

struct DressingFiberPathResult {
  bool valid = false;
  double dressing_initial = 0.0;
  double dressing_xy = 0.0;
  double dressing_yx = 0.0;
  double path_difference = 0.0;
  double predicted_holonomy = 0.0;
  double holonomy_residual = 0.0;
  double closed_loop_dressing = 0.0;
  double closed_loop_shift = 0.0;
  double reversed_loop_dressing = 0.0;
  double reversed_loop_residual = 0.0;
  double shifted_zero_holonomy_residual = 0.0;
};

DressingFiberStepResult advance_dressing_fiber(
    double dressing_before,
    const CenteredTraceWorkResult& work,
    double multiplier = 1.0);

DressingFiberPathResult compare_dressing_fiber_paths(
    double dressing_initial,
    const CuspDressingIntegrabilityResult& integrability,
    double shifted_energy_zero = 7.0);

/// Interior variation of two adjacent constraint terms with respect to their
/// shared dressing coordinate.  Stationarity requires this value to vanish.
double dressing_multiplier_stationarity(
    double previous_multiplier,
    double next_multiplier);

}  // namespace ftd::eft
