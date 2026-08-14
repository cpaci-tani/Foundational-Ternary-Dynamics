#include "ftd/eft/quartic_relative_carry_gearbox.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

long double pi() {
  return std::acos(-1.0L);
}

long double tau() {
  return 2.0L * pi();
}

struct ChartCoordinate {
  double principal = 0.0;
  std::int64_t winding = 0;
  bool valid = false;
};

ChartCoordinate split_chart(double dimensionless) {
  ChartCoordinate result;
  if (!std::isfinite(dimensionless)) return result;
  const long double value = static_cast<long double>(dimensionless);
  long double raw = std::floor((value + pi()) / tau());
  const long double lower = static_cast<long double>(
      std::numeric_limits<std::int64_t>::min());
  const long double upper = static_cast<long double>(
      std::numeric_limits<std::int64_t>::max());
  if (!std::isfinite(raw) || raw <= lower || raw >= upper) return result;

  result.winding = static_cast<std::int64_t>(raw);
  long double principal = value
      - tau() * static_cast<long double>(result.winding);
  if (principal >= pi()) {
    if (result.winding == std::numeric_limits<std::int64_t>::max()) {
      return ChartCoordinate{};
    }
    principal -= tau();
    ++result.winding;
  } else if (principal < -pi()) {
    if (result.winding == std::numeric_limits<std::int64_t>::min()) {
      return ChartCoordinate{};
    }
    principal += tau();
    --result.winding;
  }
  result.principal = static_cast<double>(principal);
  result.valid = result.principal >= -static_cast<double>(pi())
      && result.principal < static_cast<double>(pi());
  return result;
}

bool safe_add(std::int64_t left, std::int64_t right,
              std::int64_t& result) {
  if (right > 0
      && left > std::numeric_limits<std::int64_t>::max() - right) {
    return false;
  }
  if (right < 0
      && left < std::numeric_limits<std::int64_t>::min() - right) {
    return false;
  }
  result = left + right;
  return true;
}

double state_residual(const NativePairEnergyState& left,
                      const NativePairEnergyState& right) {
  return std::max(std::abs(left.coordinate - right.coordinate),
                  std::abs(left.momentum - right.momentum));
}

double maximum(double first, double second, double third = 0.0) {
  return std::max({std::abs(first), std::abs(second), std::abs(third)});
}

}  // namespace

QuarticRelativeCarryResult analyze_quartic_relative_carry_gearbox(
    const QuarticRelativeCarryInput& input) {
  QuarticRelativeCarryResult result;
  if (!std::isfinite(input.common_momentum)
      || !std::isfinite(input.momentum_scale)
      || !std::isfinite(input.tolerance)
      || !valid_native_pair_energy_state(input.relative_state)) {
    return result;
  }
  if (!(input.tolerance > 0.0)) {
    result.status = QuarticRelativeCarryStatus::InvalidTolerance;
    return result;
  }
  if (!(input.momentum_scale > 0.0)) {
    result.status = QuarticRelativeCarryStatus::InvalidMomentumScale;
    return result;
  }

  result.relative_step = advance_native_pair_energy(
      input.relative_state, input.relative_parameters);
  if (!result.relative_step.valid) {
    result.status = QuarticRelativeCarryStatus::RelativeStepFailure;
    return result;
  }

  const double root_two = std::sqrt(2.0);
  const double pi_before = input.relative_state.momentum;
  const double pi_after = result.relative_step.after.momentum;
  result.channel_momentum_before = {
      (input.common_momentum + pi_before) / root_two,
      (input.common_momentum - pi_before) / root_two,
  };
  result.channel_momentum_after = {
      (input.common_momentum + pi_after) / root_two,
      (input.common_momentum - pi_after) / root_two,
  };
  result.generated_dimensionless_increment =
      (pi_after - pi_before) / (root_two * input.momentum_scale);

  ChartCoordinate left_before = split_chart(
      result.channel_momentum_before[0] / input.momentum_scale);
  ChartCoordinate right_before = split_chart(
      result.channel_momentum_before[1] / input.momentum_scale);
  ChartCoordinate left_after = split_chart(
      result.channel_momentum_after[0] / input.momentum_scale);
  ChartCoordinate right_after = split_chart(
      result.channel_momentum_after[1] / input.momentum_scale);
  if (!left_before.valid || !right_before.valid
      || !left_after.valid || !right_after.valid) {
    result.status = QuarticRelativeCarryStatus::ChartCarryOutOfRange;
    return result;
  }
  result.channel_principal_before = {
      left_before.principal, right_before.principal};
  result.channel_principal_after = {
      left_after.principal, right_after.principal};
  result.channel_winding_before = {
      left_before.winding, right_before.winding};
  result.channel_winding_after = {
      left_after.winding, right_after.winding};

  std::int64_t aggregate_before = 0;
  if (!safe_add(left_before.winding, right_before.winding,
                aggregate_before)) {
    result.status = QuarticRelativeCarryStatus::ChartWindingOverflow;
    return result;
  }
  ReciprocalCarryInput carry_input;
  carry_input.principal_first = {left_before.principal, 0.0, 0.0};
  carry_input.principal_second = {right_before.principal, 0.0, 0.0};
  carry_input.opposite_increment = {
      result.generated_dimensionless_increment, 0.0, 0.0};
  carry_input.reciprocal_reservoir = {aggregate_before, 0, 0};
  carry_input.momentum_scale = input.momentum_scale;
  carry_input.tolerance = input.tolerance;
  result.carry_step = apply_reciprocal_carry_transaction(carry_input);
  if (!result.carry_step.valid()) {
    result.status = QuarticRelativeCarryStatus::CarryTransactionFailure;
    return result;
  }

  std::int64_t aggregate_after = 0;
  if (!safe_add(left_after.winding, right_after.winding,
                aggregate_after)) {
    result.status = QuarticRelativeCarryStatus::ChartWindingOverflow;
    return result;
  }
  result.chart_endpoint_residual = std::max(
      std::abs(result.carry_step.principal_first_after[0]
               - left_after.principal),
      std::abs(result.carry_step.principal_second_after[0]
               - right_after.principal));
  const bool aggregate_chart_matches =
      result.carry_step.reciprocal_reservoir_after[0] == aggregate_after;

  result.common_momentum_before = result.channel_momentum_before[0]
      + result.channel_momentum_before[1];
  result.common_momentum_after = result.channel_momentum_after[0]
      + result.channel_momentum_after[1];
  result.common_momentum_residual = result.common_momentum_after
      - result.common_momentum_before;
  result.relative_energy_residual = result.relative_step.energy_residual;
  const double endpoint_scale = std::max({
      1.0, maximum(left_after.principal, right_after.principal),
      maximum(result.common_momentum_before, result.common_momentum_after)});
  if (result.chart_endpoint_residual > input.tolerance * endpoint_scale
      || !aggregate_chart_matches) {
    result.status = QuarticRelativeCarryStatus::EndpointMismatch;
    return result;
  }

  auto reverse_parameters = input.relative_parameters;
  reverse_parameters.step = -reverse_parameters.step;
  const auto relative_reverse = advance_native_pair_energy(
      result.relative_step.after, reverse_parameters);
  if (!relative_reverse.valid) {
    result.status = QuarticRelativeCarryStatus::ReverseFailure;
    return result;
  }
  ReciprocalCarryInput reverse_carry_input;
  reverse_carry_input.principal_first = {
      result.carry_step.principal_first_after[0], 0.0, 0.0};
  reverse_carry_input.principal_second = {
      result.carry_step.principal_second_after[0], 0.0, 0.0};
  reverse_carry_input.opposite_increment = {
      -result.generated_dimensionless_increment, 0.0, 0.0};
  reverse_carry_input.reciprocal_reservoir = {
      result.carry_step.reciprocal_reservoir_after[0], 0, 0};
  reverse_carry_input.momentum_scale = input.momentum_scale;
  reverse_carry_input.tolerance = input.tolerance;
  const auto carry_reverse = apply_reciprocal_carry_transaction(
      reverse_carry_input);
  if (!carry_reverse.valid()) {
    result.status = QuarticRelativeCarryStatus::ReverseFailure;
    return result;
  }
  result.reverse_residual = std::max({
      state_residual(relative_reverse.after, input.relative_state),
      std::abs(carry_reverse.principal_first_after[0]
               - left_before.principal),
      std::abs(carry_reverse.principal_second_after[0]
               - right_before.principal),
  });
  const bool reverse_reservoir_matches =
      carry_reverse.reciprocal_reservoir_after[0] == aggregate_before;
  if (result.reverse_residual > input.tolerance * endpoint_scale
      || !reverse_reservoir_matches) {
    result.status = QuarticRelativeCarryStatus::ReverseFailure;
    return result;
  }

  const double gstar = std::tgamma(0.25) / std::tgamma(0.75);
  result.continuum_period_amplitude_product = std::sqrt(std::acos(-1.0))
      * gstar
      * std::sqrt(input.relative_parameters.mass
                  / (2.0 * input.relative_parameters.coupling));
  const double energy_scale = std::max({
      1.0, std::abs(result.relative_step.pair_before.hamiltonian_energy),
      std::abs(result.relative_step.pair_after.hamiltonian_energy)});
  result.relative_increment_derived_inside_selected_recursion = true;
  result.channel_impulses_equal_and_opposite =
      std::abs(
          (result.channel_momentum_after[0]
           - result.channel_momentum_before[0])
          + (result.channel_momentum_after[1]
             - result.channel_momentum_before[1]))
      <= input.tolerance * endpoint_scale;
  result.relative_energy_exact =
      std::abs(result.relative_energy_residual)
      <= 64.0 * input.relative_parameters.residual_tolerance * energy_scale;
  result.reciprocal_carry_composition_exact =
      result.carry_step.reciprocal_carry_update_exact
      && result.chart_endpoint_residual <= input.tolerance * endpoint_scale;
  result.full_state_reversal_exact = true;
  result.continuum_gstar_period_factor_exact =
      std::isfinite(result.continuum_period_amplitude_product)
      && result.continuum_period_amplitude_product > 0.0;
  if (!result.channel_impulses_equal_and_opposite
      || !result.relative_energy_exact
      || !result.reciprocal_carry_composition_exact
      || std::abs(result.common_momentum_residual)
          > input.tolerance * endpoint_scale
      || !result.continuum_gstar_period_factor_exact) {
    result.status = QuarticRelativeCarryStatus::EndpointMismatch;
    return result;
  }

  result.status = QuarticRelativeCarryStatus::Valid;
  return result;
}

}  // namespace ftd::eft
