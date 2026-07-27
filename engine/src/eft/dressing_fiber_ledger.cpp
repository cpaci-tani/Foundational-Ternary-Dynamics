#include "ftd/eft/dressing_fiber_ledger.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

double sign_nonzero(double value) {
  return value < 0.0 ? -1.0 : value > 0.0 ? 1.0 : 0.0;
}

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

}  // namespace

DressingFiberStepResult advance_dressing_fiber(
    double dressing_before,
    const CenteredTraceWorkResult& work,
    double multiplier) {
  DressingFiberStepResult result;
  result.dressing_before = dressing_before;
  result.multiplier = multiplier;
  if (!work.valid || !std::isfinite(dressing_before)
      || !std::isfinite(multiplier)) {
    return result;
  }

  result.dressing_change = work.omitted_work;
  result.dressing_after = dressing_before + result.dressing_change;
  result.field_energy_change = -work.field_work;
  result.matter_energy_change = work.centered_work;
  result.extended_energy_residual = std::abs(
      result.field_energy_change + result.matter_energy_change
      + result.dressing_change);
  const double dressing_reversed = result.dressing_after
      - result.dressing_change;
  result.reverse_dressing_residual = std::abs(
      dressing_reversed - dressing_before);

  // S_constraint=lambda[W_cusp-(D1-D0)].
  result.constraint_derivative_D0 = multiplier;
  result.constraint_derivative_D1 = -multiplier;
  const double half_gq = 0.5 * work.coupling
      * static_cast<double>(work.charge);
  result.cusp_position_gradient = {
      half_gq * work.jump.x * sign_nonzero(work.displacement.x),
      half_gq * work.jump.y * sign_nonzero(work.displacement.y),
      half_gq * work.jump.z * sign_nonzero(work.displacement.z)};
  result.constraint_position_gradient =
      result.cusp_position_gradient * multiplier;
  result.action_gradient_residual = max_difference(
      result.constraint_position_gradient,
      result.cusp_position_gradient * multiplier);
  result.valid = std::isfinite(result.dressing_after)
      && std::isfinite(result.extended_energy_residual)
      && std::isfinite(result.action_gradient_residual);
  return result;
}

DressingFiberPathResult compare_dressing_fiber_paths(
    double dressing_initial,
    const CuspDressingIntegrabilityResult& integrability,
    double shifted_energy_zero) {
  DressingFiberPathResult result;
  result.dressing_initial = dressing_initial;
  if (!integrability.valid || !std::isfinite(dressing_initial)
      || !std::isfinite(shifted_energy_zero)) {
    return result;
  }
  result.dressing_xy = dressing_initial + integrability.path_xy;
  result.dressing_yx = dressing_initial + integrability.path_yx;
  result.path_difference = result.dressing_xy - result.dressing_yx;
  result.predicted_holonomy = integrability.plaquette_holonomy_xy;
  result.holonomy_residual = std::abs(
      result.path_difference - result.predicted_holonomy);

  // Traverse xy, then the reverse of yx, returning to the initial site.
  result.closed_loop_dressing = result.dressing_xy
      - integrability.path_yx;
  result.closed_loop_shift = result.closed_loop_dressing
      - dressing_initial;
  // Reverse that entire oriented loop.
  result.reversed_loop_dressing = result.closed_loop_dressing
      - integrability.path_xy + integrability.path_yx;
  result.reversed_loop_residual = std::abs(
      result.reversed_loop_dressing - dressing_initial);

  const double shifted_xy = dressing_initial + shifted_energy_zero
      + integrability.path_xy;
  const double shifted_yx = dressing_initial + shifted_energy_zero
      + integrability.path_yx;
  result.shifted_zero_holonomy_residual = std::abs(
      (shifted_xy - shifted_yx) - result.path_difference);
  result.valid = std::isfinite(result.closed_loop_shift)
      && std::isfinite(result.reversed_loop_residual);
  return result;
}

double dressing_multiplier_stationarity(
    double previous_multiplier,
    double next_multiplier) {
  return next_multiplier - previous_multiplier;
}

}  // namespace ftd::eft
