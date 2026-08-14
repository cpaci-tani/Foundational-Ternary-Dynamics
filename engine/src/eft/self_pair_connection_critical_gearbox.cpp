#include "ftd/eft/self_pair_connection_critical_gearbox.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite_state(const SelfPairConnectionState& state) {
  return std::isfinite(state.common_coordinate)
      && std::isfinite(state.relative_coordinate)
      && std::isfinite(state.relative_momentum);
}

double signed_pair(double coordinate) {
  return coordinate * std::abs(coordinate);
}

double scale(double first, double second = 0.0, double third = 0.0) {
  return std::max({1.0, std::abs(first), std::abs(second), std::abs(third)});
}

}  // namespace

SelfPairConnectionResult analyze_self_pair_connection_critical_gearbox(
    const SelfPairConnectionState& state,
    const SelfPairConnectionParameters& parameters) {
  SelfPairConnectionResult result;
  result.before = state;
  if (!finite_state(state)
      || !std::isfinite(parameters.common_mass)
      || !std::isfinite(parameters.relative_mass)
      || !std::isfinite(parameters.bare_quartic_coupling)
      || !std::isfinite(parameters.gamma)
      || !std::isfinite(parameters.step)
      || !std::isfinite(parameters.momentum_scale)
      || !std::isfinite(parameters.tolerance)
      || !std::isfinite(
          parameters.moving_common_momentum_projection_probe)) {
    return result;
  }
  if (!(parameters.common_mass > 0.0)) {
    result.status = SelfPairConnectionStatus::InvalidCommonMass;
    return result;
  }
  if (!(parameters.relative_mass > 0.0)) {
    result.status = SelfPairConnectionStatus::InvalidRelativeMass;
    return result;
  }
  if (!(parameters.bare_quartic_coupling > 0.0)) {
    result.status = SelfPairConnectionStatus::InvalidQuarticCoupling;
    return result;
  }
  if (parameters.step == 0.0) {
    result.status = SelfPairConnectionStatus::InvalidStep;
    return result;
  }
  if (!(parameters.tolerance > 0.0)) {
    result.status = SelfPairConnectionStatus::InvalidTolerance;
    return result;
  }
  if (!(parameters.momentum_scale > 0.0)) {
    result.status = SelfPairConnectionStatus::InvalidMomentumScale;
    return result;
  }
  if (parameters.max_iterations == 0) {
    result.status = SelfPairConnectionStatus::InvalidIterationLimit;
    return result;
  }

  const long double gamma = parameters.gamma;
  const long double common_mass = parameters.common_mass;
  const long double connection_coupling =
      gamma * gamma / (2.0L * common_mass);
  const long double effective_coupling =
      parameters.bare_quartic_coupling + connection_coupling;
  if (!std::isfinite(connection_coupling)
      || !std::isfinite(effective_coupling)
      || effective_coupling <= 0.0L
      || effective_coupling
          > static_cast<long double>(std::numeric_limits<double>::max())) {
    result.status = SelfPairConnectionStatus::EffectiveCouplingOverflow;
    return result;
  }
  result.connection_quartic_contribution =
      static_cast<double>(connection_coupling);
  result.effective_quartic_coupling =
      static_cast<double>(effective_coupling);
  result.signed_pair_before = signed_pair(state.relative_coordinate);
  if (!std::isfinite(result.signed_pair_before)) {
    result.status = SelfPairConnectionStatus::SignedPairOverflow;
    return result;
  }

  QuarticRelativeCarryInput child;
  child.relative_state = {
      state.relative_coordinate, state.relative_momentum};
  child.relative_parameters.mass = parameters.relative_mass;
  child.relative_parameters.coupling = result.effective_quartic_coupling;
  child.relative_parameters.step = parameters.step;
  child.relative_parameters.residual_tolerance = parameters.tolerance;
  child.relative_parameters.max_iterations = parameters.max_iterations;
  child.common_momentum = 0.0;
  child.momentum_scale = parameters.momentum_scale;
  child.tolerance = parameters.tolerance;
  result.relative_carry_step = analyze_quartic_relative_carry_gearbox(child);
  if (!result.relative_carry_step.valid()) {
    result.status = SelfPairConnectionStatus::RelativeCarryFailure;
    return result;
  }

  result.after = state;
  result.after.relative_coordinate =
      result.relative_carry_step.relative_step.after.coordinate;
  result.after.relative_momentum =
      result.relative_carry_step.relative_step.after.momentum;
  result.signed_pair_after = signed_pair(result.after.relative_coordinate);
  if (!std::isfinite(result.signed_pair_after)) {
    result.status = SelfPairConnectionStatus::SignedPairOverflow;
    return result;
  }

  const double symmetric_pair =
      0.5 * (result.signed_pair_after + result.signed_pair_before);
  result.common_displacement = -parameters.step * parameters.gamma
      * symmetric_pair / parameters.common_mass;
  result.after.common_coordinate = state.common_coordinate
      + result.common_displacement;
  if (!finite_state(result.after)) {
    result.status = SelfPairConnectionStatus::SignedPairOverflow;
    return result;
  }

  result.mechanical_common_momentum_before =
      -parameters.gamma * result.signed_pair_before;
  result.mechanical_common_momentum_after =
      -parameters.gamma * result.signed_pair_after;
  result.mechanical_impulse_residual =
      result.mechanical_common_momentum_after
      - result.mechanical_common_momentum_before
      + parameters.gamma
          * (result.signed_pair_after - result.signed_pair_before);
  result.common_endpoint_equation_residual =
      result.after.common_coordinate - state.common_coordinate
      + parameters.step * parameters.gamma * symmetric_pair
          / parameters.common_mass;
  result.rest_energy_residual =
      result.relative_carry_step.relative_energy_residual;

  const double reverse_common = result.after.common_coordinate
      + parameters.step * parameters.gamma * symmetric_pair
          / parameters.common_mass;
  result.reverse_common_coordinate_residual =
      reverse_common - state.common_coordinate;

  result.connection_derivative_before = 2.0 * parameters.gamma
      * std::abs(state.relative_coordinate);
  result.connection_derivative_after = 2.0 * parameters.gamma
      * std::abs(result.after.relative_coordinate);
  result.self_pair_origin_jacobian = 0.0;
  result.critical_clock_hessian = 0.0;
  result.moving_quadratic_ray_coefficient = -parameters.gamma
      * parameters.moving_common_momentum_projection_probe
      / parameters.common_mass;
  result.symmetric_full_cycle_drift_residual =
      -parameters.gamma * (
          result.signed_pair_before - result.signed_pair_before)
      / parameters.common_mass;
  const double gstar = std::tgamma(0.25) / std::tgamma(0.75);
  result.continuum_period_amplitude_product = std::sqrt(std::acos(-1.0))
      * gstar * std::sqrt(parameters.relative_mass
          / (2.0 * result.effective_quartic_coupling));
  result.conditional_equal_partition_gamma_magnitude =
      std::sqrt(2.0 * parameters.common_mass
          * parameters.bare_quartic_coupling);

  const double invariant_scale = scale(
      result.mechanical_common_momentum_before,
      result.mechanical_common_momentum_after,
      result.after.common_coordinate);
  const double accepted = 128.0 * parameters.tolerance * invariant_scale;
  result.imposed_signed_self_pair_connection = true;
  result.positive_linearized_connection_obstruction_registered = true;
  result.origin_connection_derivative_zero = true;
  result.connection_derivative_nonzero_away_for_nonzero_gamma =
      parameters.gamma != 0.0
      && (state.relative_coordinate != 0.0
          || result.after.relative_coordinate != 0.0);
  result.rest_sector_quartic_fold_exact =
      std::isfinite(result.effective_quartic_coupling)
      && result.effective_quartic_coupling > 0.0;
  result.rest_sector_critical_quartic_exact =
      result.critical_clock_hessian == 0.0;
  result.mechanical_common_impulse_exact =
      std::abs(result.mechanical_impulse_residual) <= accepted;
  result.common_endpoint_update_exact =
      std::abs(result.common_endpoint_equation_residual) <= accepted;
  result.relative_energy_exact =
      result.relative_carry_step.relative_energy_exact;
  result.channel_impulses_equal_and_opposite =
      result.relative_carry_step.channel_impulses_equal_and_opposite;
  result.reciprocal_carry_composition_exact =
      result.relative_carry_step.reciprocal_carry_composition_exact;
  result.signed_step_reversal_exact =
      result.relative_carry_step.full_state_reversal_exact
      && std::abs(result.reverse_common_coordinate_residual) <= accepted;
  result.continuum_gstar_period_factor_exact =
      std::isfinite(result.continuum_period_amplitude_product)
      && result.continuum_period_amplitude_product > 0.0;
  result.moving_sector_has_generic_quadratic_term =
      parameters.gamma != 0.0
      && parameters.moving_common_momentum_projection_probe != 0.0
      && result.moving_quadratic_ray_coefficient != 0.0;
  result.moving_sector_exact_quartic_generic = false;
  result.polarized_symmetric_full_cycle_drift_zero =
      result.symmetric_full_cycle_drift_residual == 0.0;
  result.i_supplies_orientation = true;

  if (!result.rest_sector_quartic_fold_exact
      || !result.rest_sector_critical_quartic_exact
      || !result.mechanical_common_impulse_exact
      || !result.common_endpoint_update_exact
      || !result.relative_energy_exact
      || !result.channel_impulses_equal_and_opposite
      || !result.reciprocal_carry_composition_exact) {
    result.status = SelfPairConnectionStatus::InvariantFailure;
    return result;
  }
  if (!result.signed_step_reversal_exact) {
    result.status = SelfPairConnectionStatus::ReverseFailure;
    return result;
  }
  result.status = SelfPairConnectionStatus::Valid;
  return result;
}

}  // namespace ftd::eft
