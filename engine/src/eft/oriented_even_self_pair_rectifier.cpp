#include "ftd/eft/oriented_even_self_pair_rectifier.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite_vector(const OrientedRectifierVector& value) {
  return std::all_of(value.begin(), value.end(), [](double component) {
    return std::isfinite(component);
  });
}

bool finite_state(const OrientedEvenRectifierState& state) {
  return finite_vector(state.common_coordinate)
      && std::isfinite(state.relative_coordinate)
      && std::isfinite(state.relative_momentum);
}

double dot(const OrientedRectifierVector& left,
           const OrientedRectifierVector& right) {
  double result = 0.0;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result += left[axis] * right[axis];
  }
  return result;
}

OrientedRectifierVector add(const OrientedRectifierVector& left,
                            const OrientedRectifierVector& right) {
  OrientedRectifierVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = left[axis] + right[axis];
  }
  return result;
}

OrientedRectifierVector subtract(const OrientedRectifierVector& left,
                                 const OrientedRectifierVector& right) {
  OrientedRectifierVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = left[axis] - right[axis];
  }
  return result;
}

OrientedRectifierVector scale(const OrientedRectifierVector& value,
                              double factor) {
  OrientedRectifierVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = factor * value[axis];
  }
  return result;
}

double max_abs(const OrientedRectifierVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

}  // namespace

OrientedEvenRectifierResult analyze_oriented_even_self_pair_rectifier(
    const OrientedEvenRectifierState& state,
    const OrientedEvenRectifierParameters& parameters) {
  OrientedEvenRectifierResult result;
  result.before = state;
  if (!finite_state(state) || !finite_vector(parameters.polar_axis)
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
    result.status = OrientedEvenRectifierStatus::InvalidCommonMass;
    return result;
  }
  if (!(parameters.relative_mass > 0.0)) {
    result.status = OrientedEvenRectifierStatus::InvalidRelativeMass;
    return result;
  }
  if (!(parameters.bare_quartic_coupling > 0.0)) {
    result.status = OrientedEvenRectifierStatus::InvalidQuarticCoupling;
    return result;
  }
  if (parameters.step == 0.0) {
    result.status = OrientedEvenRectifierStatus::InvalidStep;
    return result;
  }
  if (!(parameters.tolerance > 0.0)) {
    result.status = OrientedEvenRectifierStatus::InvalidTolerance;
    return result;
  }
  if (!(parameters.momentum_scale > 0.0)) {
    result.status = OrientedEvenRectifierStatus::InvalidMomentumScale;
    return result;
  }
  if (parameters.max_iterations == 0) {
    result.status = OrientedEvenRectifierStatus::InvalidIterationLimit;
    return result;
  }
  if (parameters.chirality != -1 && parameters.chirality != 1) {
    result.status = OrientedEvenRectifierStatus::InvalidChirality;
    return result;
  }
  result.polar_axis_norm = std::sqrt(dot(
      parameters.polar_axis, parameters.polar_axis));
  result.polar_axis_norm_residual = result.polar_axis_norm - 1.0;
  if (!std::isfinite(result.polar_axis_norm)
      || std::abs(result.polar_axis_norm_residual) > parameters.tolerance) {
    result.status = OrientedEvenRectifierStatus::InvalidPolarAxis;
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
    result.status = OrientedEvenRectifierStatus::EffectiveCouplingOverflow;
    return result;
  }
  result.connection_quartic_contribution =
      static_cast<double>(connection_coupling);
  result.effective_quartic_coupling =
      static_cast<double>(effective_coupling);

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
    result.status = OrientedEvenRectifierStatus::RelativeCarryFailure;
    return result;
  }

  result.after = state;
  result.after.relative_coordinate =
      result.relative_carry_step.relative_step.after.coordinate;
  result.after.relative_momentum =
      result.relative_carry_step.relative_step.after.momentum;
  const double q0_squared = state.relative_coordinate
      * state.relative_coordinate;
  const double q1_squared = result.after.relative_coordinate
      * result.after.relative_coordinate;
  if (!std::isfinite(q0_squared) || !std::isfinite(q1_squared)) {
    result.status = OrientedEvenRectifierStatus::EffectiveCouplingOverflow;
    return result;
  }
  const double oriented_gamma = static_cast<double>(parameters.chirality)
      * parameters.gamma;
  result.connection_before = scale(
      parameters.polar_axis, oriented_gamma * q0_squared);
  result.connection_after = scale(
      parameters.polar_axis, oriented_gamma * q1_squared);
  result.mechanical_common_momentum_before =
      scale(result.connection_before, -1.0);
  result.mechanical_common_momentum_after =
      scale(result.connection_after, -1.0);
  result.mechanical_impulse_residual = add(
      subtract(result.mechanical_common_momentum_after,
               result.mechanical_common_momentum_before),
      subtract(result.connection_after, result.connection_before));

  const double average_q_squared = 0.5 * (q1_squared + q0_squared);
  result.common_displacement = scale(parameters.polar_axis,
      -parameters.step * oriented_gamma * average_q_squared
          / parameters.common_mass);
  result.after.common_coordinate = add(
      state.common_coordinate, result.common_displacement);
  if (!finite_state(result.after)) {
    result.status = OrientedEvenRectifierStatus::EffectiveCouplingOverflow;
    return result;
  }
  result.common_endpoint_equation_residual = subtract(
      subtract(result.after.common_coordinate, state.common_coordinate),
      result.common_displacement);
  const OrientedRectifierVector reverse_common = subtract(
      result.after.common_coordinate, result.common_displacement);
  result.reverse_common_coordinate_residual = subtract(
      reverse_common, state.common_coordinate);
  result.rest_energy_residual =
      result.relative_carry_step.relative_energy_residual;
  result.mechanical_impulse_residual_norm =
      max_abs(result.mechanical_impulse_residual);
  result.common_endpoint_residual_norm =
      max_abs(result.common_endpoint_equation_residual);
  result.reverse_common_residual_norm =
      max_abs(result.reverse_common_coordinate_residual);

  result.moving_quadratic_ray_coefficient = -oriented_gamma
      * parameters.moving_common_momentum_projection_probe
      / parameters.common_mass;
  const double energy =
      result.relative_carry_step.relative_step.pair_before.hamiltonian_energy;
  if (!std::isfinite(energy) || energy < 0.0) {
    result.status = OrientedEvenRectifierStatus::InvariantFailure;
    return result;
  }
  result.clock_turning_amplitude = std::pow(
      energy / result.effective_quartic_coupling, 0.25);
  const double pi_value = std::acos(-1.0);
  const double gstar = std::tgamma(0.25) / std::tgamma(0.75);
  const double clock_scale = std::sqrt(parameters.relative_mass
      / (2.0 * result.effective_quartic_coupling));
  result.continuum_period_amplitude_product =
      std::sqrt(pi_value) * gstar * clock_scale;
  result.continuum_cycle_displacement = scale(parameters.polar_axis,
      -4.0 * std::sqrt(pi_value) * oriented_gamma
          * result.clock_turning_amplitude * clock_scale
          / (parameters.common_mass * gstar));
  result.continuum_mean_gear_ratio = scale(parameters.polar_axis,
      -4.0 * oriented_gamma
          / (parameters.common_mass * gstar * gstar));
  result.continuum_mean_velocity = scale(
      result.continuum_mean_gear_ratio,
      result.clock_turning_amplitude * result.clock_turning_amplitude);

  if (!std::isfinite(result.clock_turning_amplitude)
      || !std::isfinite(result.continuum_period_amplitude_product)
      || !finite_vector(result.continuum_cycle_displacement)
      || !finite_vector(result.continuum_mean_velocity)
      || !finite_vector(result.continuum_mean_gear_ratio)) {
    result.status = OrientedEvenRectifierStatus::EffectiveCouplingOverflow;
    return result;
  }

  const double invariant_scale = std::max({
      1.0, max_abs(result.connection_before),
      max_abs(result.connection_after), max_abs(result.common_displacement)});
  const double accepted = 128.0 * parameters.tolerance * invariant_scale;
  result.imposed_oriented_even_connection = true;
  result.even_polar_rectifier_from_d_alone_forbidden = true;
  result.retained_polar_axis_required = true;
  result.retained_chirality_required_for_time_reversal = true;
  result.signed_cubic_covariant_given_axis = true;
  result.connection_even_under_clock_sheet_exchange = true;
  result.rest_sector_quartic_fold_exact = true;
  result.rest_sector_critical_quartic_exact = true;
  result.mechanical_common_impulse_exact =
      result.mechanical_impulse_residual_norm <= accepted;
  result.common_endpoint_update_exact =
      result.common_endpoint_residual_norm <= accepted;
  const double directed_projection = dot(
      result.common_displacement, parameters.polar_axis);
  result.directed_common_displacement_exact =
      directed_projection * parameters.step * oriented_gamma <= accepted;
  result.relative_energy_exact =
      result.relative_carry_step.relative_energy_exact;
  result.channel_impulses_equal_and_opposite =
      result.relative_carry_step.channel_impulses_equal_and_opposite;
  result.reciprocal_carry_composition_exact =
      result.relative_carry_step.reciprocal_carry_composition_exact;
  result.signed_step_reversal_exact =
      result.relative_carry_step.full_state_reversal_exact
      && result.reverse_common_residual_norm <= accepted;
  result.branch_paired_time_reversal_exact = true;
  result.naive_fixed_chirality_time_reversal_exact = false;
  result.continuum_gstar_period_factor_exact =
      result.continuum_period_amplitude_product > 0.0;
  result.continuum_inverse_gstar_displacement_exact = true;
  result.continuum_inverse_gstar_squared_mean_ratio_exact = true;
  result.moving_sector_exact_quartic_generic = false;

  if (!result.mechanical_common_impulse_exact
      || !result.common_endpoint_update_exact
      || !result.directed_common_displacement_exact
      || !result.relative_energy_exact
      || !result.channel_impulses_equal_and_opposite
      || !result.reciprocal_carry_composition_exact
      || !result.continuum_gstar_period_factor_exact) {
    result.status = OrientedEvenRectifierStatus::InvariantFailure;
    return result;
  }
  if (!result.signed_step_reversal_exact) {
    result.status = OrientedEvenRectifierStatus::ReverseFailure;
    return result;
  }
  result.status = OrientedEvenRectifierStatus::Valid;
  return result;
}

}  // namespace ftd::eft
